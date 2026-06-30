import os
import sys
import hashlib
import json
import base64
import urllib.request
import urllib.error
import threading
import subprocess
import time
import ssl
import re
import logging
import traceback
import queue

# === FASE 1: Logging e handler global ANTES de qualquer outro import ===
LOG_DIR = None  # Será definido após get_exe_dir()
LOG_PATH = None

def setup_logging():
    """Configura logging em arquivo. Chamado após get_exe_dir() estar disponível."""
    global LOG_DIR, LOG_PATH
    LOG_DIR = get_exe_dir()
    LOG_PATH = os.path.join(LOG_DIR, "launcher.log")
    try:
        logging.basicConfig(
            filename=LOG_PATH,
            level=logging.DEBUG,
            format='%(asctime)s [%(levelname)s] %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S',
            force=True
        )
        logging.info("=== Launcher iniciado ===")
    except Exception:
        pass

def global_exception_handler(exc_type, exc_value, exc_tb):
    """Captura qualquer exceção não tratada, loga e exibe ao usuário."""
    tb_text = ''.join(traceback.format_exception(exc_type, exc_value, exc_tb))
    log_msg = f"Exceção não tratada: {exc_value}\n{tb_text}"
    logging.error(log_msg)
    try:
        import tkinter as _tk
        from tkinter import messagebox as _mb
        _root = _tk.Tk()
        _root.withdraw()
        _mb.showerror(
            "Launcher — Erro Crítico",
            f"O launcher encontrou um erro inesperado.\n\n"
            f"Detalhes salvos em:\n{LOG_PATH or 'launcher.log'}\n\n"
            f"Erro: {exc_value}"
        )
        _root.destroy()
    except Exception:
        pass
    sys.exit(1)

sys.excepthook = global_exception_handler

# === Imports com proteção (FASE 1) ===
try:
    import certifi
    HAS_CERTIFI = True
except ImportError:
    HAS_CERTIFI = False
    logging.warning("certifi não encontrado — usando SSL padrão do sistema")

try:
    from PIL import Image, ImageTk
    HAS_PIL = True
except ImportError:
    HAS_PIL = False
    logging.warning("Pillow (PIL) não encontrado — background não será exibido")
    # Não fatal — launcher funciona sem imagem de fundo

from concurrent.futures import ThreadPoolExecutor, as_completed

# Proteção de tkinter — se falhar, o launcher não tem como mostrar nada
try:
    import tkinter as tk
    from tkinter import messagebox
except ImportError:
    print("ERRO: tkinter não disponível. Instale python3-tk no Linux ou use Python oficial no Windows.")
    sys.exit(1)

import ctypes

# ============================================================
# CONSTANTES E CONFIGURAÇÕES
# ============================================================

def is_admin():
    """Verifica se o processo atual possui privilégios de Administrador no Windows."""
    try:
        return ctypes.windll.shell32.IsUserAnAdmin()
    except Exception:
        return False

TEXT_EXTENSIONS = {".txt", ".ini", ".bat", ".sqf", ".c", ".inc", ".xml", ".cfg", ".layout", ".gproj", ".json", ".html", ".css", ".js", ".h", ".qss", ".sproj", ".ssln"}
LOCAL_MUTABLE_FILES = {"!Start_client_parameters.ini", "DayZSetting.xml"}
CACHE_SCHEMA_VERSION = 6
IGNORED_MANIFEST_EXTENSIONS = {".bisign"}
IGNORED_MANIFEST_EXACT_RELPATHS = {
    "Launcher.exe",
    "launcher_config.json",
    "launcher_cache.json",
    "DayZSetting.xml",
    "PLANO_OTIMIZACAO_PERFORMANCE.md",
}
IGNORED_MANIFEST_NAME_MARKERS = (".backup_", ".bak-", ".bak_", ".old")
NON_CRITICAL_EXTENSIONS = {".png", ".jpg", ".ico", ".wav", ".chm", ".chw", ".chi", ".html", ".css"}
DOWNLOAD_SPEED_LIMIT_MBPS = 0  # 0 = sem limite; ajustar conforme necessidade
MIN_DISK_SPACE_MB = 2048  # Espaço mínimo requerido em MB

def should_ignore_manifest_entry(relpath):
    normalized = relpath.replace("\\", "/")
    lowered = normalized.lower()
    basename = os.path.basename(lowered)
    return (
        normalized in IGNORED_MANIFEST_EXACT_RELPATHS
        or os.path.splitext(lowered)[1] in IGNORED_MANIFEST_EXTENSIONS
        or any(marker in basename for marker in IGNORED_MANIFEST_NAME_MARKERS)
        or lowered.endswith(".download")
        or lowered.endswith(".tmp")
        or lowered.endswith(".lst")
        or lowered.endswith(".log")
        or lowered.endswith(".rpt")
        or lowered.endswith(".dmp")
        or lowered.endswith(".mdmp")
        or lowered.endswith(".bidmp")
    )

# === FASE 2: Cache de classificação de arquivos ===
class FileClassifier:
    """Cache de classificação binário/texto por filepath para evitar I/O duplo."""
    def __init__(self):
        self._cache = {}

    def is_text_file(self, filepath):
        if filepath in self._cache:
            return self._cache[filepath]
        result = self._is_text_file_impl(filepath)
        self._cache[filepath] = result
        return result

    def _is_text_file_impl(self, filepath):
        ext = os.path.splitext(filepath)[1].lower()
        if ext in {".pbo", ".dll", ".exe", ".png", ".jpg", ".ico", ".wav", ".zip",
                    ".rar", ".db", ".paa", ".p3d", ".rtm", ".wrp", ".ogg", ".bisign", ".bik"}:
            return False
        if re.search(r'\.part\d+$', ext):
            return False
        try:
            if not os.path.exists(filepath):
                return False
            size = os.path.getsize(filepath)
            if size == 0 or size > 10 * 1024 * 1024:
                return False
            with open(filepath, 'rb') as f:
                chunk = f.read(8192)
                return b'\x00' not in chunk
        except Exception:
            return False

    def clear(self):
        self._cache.clear()

# Instância global do classificador
_file_classifier = FileClassifier()

def is_text_file(filepath):
    """Wrapper global que usa o cache."""
    return _file_classifier.is_text_file(filepath)

def get_file_info(filepath):
    """Retorna (tamanho, md5) do arquivo. Se for texto, normaliza para LF line endings antes de calcular."""
    if is_text_file(filepath):
        try:
            with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
            normalized = content.replace("\r\n", "\n").encode("utf-8")
            md5_hash = hashlib.md5(normalized).hexdigest()
            size = len(normalized)
            return size, md5_hash
        except Exception:
            pass

    # Fallback para binário — chunk de 1MB (FASE 2: unificado)
    hash_md5 = hashlib.md5()
    try:
        size = os.path.getsize(filepath)
        with open(filepath, "rb") as f:
            for chunk in iter(lambda: f.read(1024 * 1024), b""):
                hash_md5.update(chunk)
        return size, hash_md5.hexdigest()
    except Exception:
        return None, None

def get_file_integrity_variants(filepath, logical_path=None):
    """Retorna pares (tamanho, md5) aceitos para comparar manifesto atual e manifestos legados."""
    if is_text_file(filepath):
        try:
            with open(filepath, "r", encoding="utf-8", errors="ignore") as f:
                content = f.read()
            lf_bytes = content.replace("\r\n", "\n").encode("utf-8")
            crlf_bytes = content.replace("\r\n", "\n").replace("\n", "\r\n").encode("utf-8")
            variants = [(len(lf_bytes), hashlib.md5(lf_bytes).hexdigest())]
            crlf_variant = (len(crlf_bytes), hashlib.md5(crlf_bytes).hexdigest())
            if crlf_variant not in variants:
                variants.append(crlf_variant)
            return variants
        except Exception:
            pass

    size, md5_hash = get_file_info(filepath)
    if size is None or md5_hash is None:
        return []
    return [(size, md5_hash)]

def relaunch_without_console_if_needed():
    """No Windows, se iniciado via python.exe em modo console, relança com pythonw.exe."""
    if os.name != "nt":
        return False
    if getattr(sys, 'frozen', False):
        return False
    try:
        executable = os.path.basename(sys.executable).lower()
        if executable != "python.exe":
            return False
        pythonw = os.path.join(os.path.dirname(sys.executable), "pythonw.exe")
        if not os.path.exists(pythonw):
            return False
        creationflags = getattr(subprocess, "CREATE_NO_WINDOW", 0)
        subprocess.Popen([pythonw] + sys.argv, cwd=get_exe_dir(), creationflags=creationflags)
        return True
    except Exception:
        return False

# Configurações do Repositório do GitHub
# Repositório dedicado exclusivamente ao client; arquivos via raw, manifesto via API para evitar cache antigo do CDN.
GITHUB_RAW_URL = "https://raw.githubusercontent.com/Stoltemberg/dayz-client/main/"
GITHUB_MANIFEST_API_URL = "https://api.github.com/repos/Stoltemberg/dayz-client/contents/manifest.json?ref=main"
MANIFEST_NAME = "manifest.json"

# IP usado pelo host local ao clicar em JOGAR neste PC.
# O Radmin VPN continua documentado para jogadores externos; o host deve usar
# loopback para evitar que o cliente fique preso antes do handshake local.
SERVER_IP = "127.0.0.1"

# Cores do Tema Premium Dark & Red
BG_COLOR = "#0b0b0e"
CARD_COLOR = "#121217"
ACCENT_COLOR = "#e51b24"
ACCENT_HOVER = "#ff333d"
TEXT_MAIN = "#f5f5f7"
TEXT_MUTED = "#9a9aa2"
BORDER_COLOR = "#2a2a33"
PROGRESS_BG = "#1e1e24"

def get_exe_dir():
    """Obtém o diretório onde o executável final ou o script principal reside."""
    if getattr(sys, 'frozen', False):
        return os.path.dirname(sys.argv[0])
    return os.path.dirname(os.path.abspath(__file__))

def check_disk_space(path, required_mb=MIN_DISK_SPACE_MB):
    """Verifica se há espaço em disco suficiente. Retorna (ok, free_mb)."""
    try:
        if sys.platform == "win32":
            import shutil
            total, used, free = shutil.disk_usage(path)
            free_mb = free / (1024 * 1024)
            return free_mb >= required_mb, free_mb
        else:
            stat = os.statvfs(path)
            free_mb = (stat.f_bavail * stat.f_frsize) / (1024 * 1024)
            return free_mb >= required_mb, free_mb
    except Exception as e:
        logging.error(f"Erro ao verificar espaço em disco: {e}")
        return True, 0  # Em caso de erro, não bloqueia

def resource_path(relative_path):
    """Obtém caminho absoluto para recursos com múltiplos fallbacks."""
    if hasattr(sys, '_MEIPASS'):
        base_path = sys._MEIPASS
        path = os.path.join(base_path, relative_path)
        if os.path.exists(path):
            return path
    exe_dir = get_exe_dir()
    path = os.path.join(exe_dir, relative_path)
    if os.path.exists(path):
        return path
    path = os.path.join(os.getcwd(), relative_path)
    if os.path.exists(path):
        return path
    return os.path.join(exe_dir, relative_path)

def setup_windows_app_id():
    """Define AppUserModelID no Windows para corrigir ícone na barra de tarefas."""
    if sys.platform == "win32":
        try:
            ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID("Stoltemberg.DayZ.Launcher")
        except Exception:
            pass


def force_taskbar_icon(window, refresh_visibility=True):
    """Força janela Tkinter borderless a aparecer na barra de tarefas do Windows.

    Importante: o refresh por withdraw/deiconify só pode acontecer de forma
    controlada. Ele dispara eventos <Map>; se for chamado dentro do próprio
    <Map>, cria loop de abre/fecha.
    """
    if sys.platform != "win32":
        return
    try:
        window.update_idletasks()
        hwnd = ctypes.windll.user32.GetParent(window.winfo_id())
        if not hwnd:
            hwnd = window.winfo_id()
        GWL_EXSTYLE = -20
        WS_EX_TOOLWINDOW = 0x00000080
        WS_EX_APPWINDOW = 0x00040000
        style = ctypes.windll.user32.GetWindowLongW(hwnd, GWL_EXSTYLE)
        style = (style & ~WS_EX_TOOLWINDOW) | WS_EX_APPWINDOW
        ctypes.windll.user32.SetWindowLongW(hwnd, GWL_EXSTYLE, style)

        if refresh_visibility and not getattr(window, "_taskbar_refreshing", False):
            window._taskbar_refreshing = True
            window.withdraw()
            def _show_again():
                try:
                    window.deiconify()
                    window.lift()
                finally:
                    window._taskbar_refreshing = False
            window.after(80, _show_again)
    except Exception as e:
        try:
            window._taskbar_refreshing = False
        except Exception:
            pass
        logging.warning(f"Erro ao forçar ícone na barra de tarefas: {e}")


def minimize_to_taskbar(window):
    """Minimiza corretamente janela borderless para a barra de tarefas."""
    try:
        window._minimizing_to_taskbar = True
        window.overrideredirect(False)
        force_taskbar_icon(window, refresh_visibility=False)
        window.iconify()
    except Exception as e:
        logging.warning(f"Erro ao minimizar launcher: {e}")


def restore_borderless_window(window):
    """Restaura o modo borderless depois que a janela volta da taskbar."""
    try:
        if getattr(window, "_taskbar_refreshing", False):
            return
        if window.state() == "normal":
            window._minimizing_to_taskbar = False
            window.overrideredirect(True)
            force_taskbar_icon(window, refresh_visibility=False)
    except Exception as e:
        logging.warning(f"Erro ao restaurar launcher: {e}")

# Configurações de caminhos locais
CONFIG_PATH = os.path.join(get_exe_dir(), "launcher_config.json")
CACHE_PATH = os.path.join(get_exe_dir(), "launcher_cache.json")
LOCAL_MANIFEST_PATH = resource_path("manifest.json")

# Inicializa logging agora que get_exe_dir() existe
setup_logging()

# === FASE 3: Rastreador de velocidade de download ===
class SpeedTracker:
    """Mede velocidade instantânea de download em MB/s e calcula ETA."""
    def __init__(self, window=5.0):
        self._samples = []
        self._window = window
        self._total_bytes = 0

    def update(self, bytes_downloaded):
        now = time.time()
        self._samples.append((now, bytes_downloaded))
        self._total_bytes = bytes_downloaded
        cutoff = now - self._window
        self._samples = [(t, b) for t, b in self._samples if t > cutoff]

    def get_speed_bytes_per_sec(self):
        if len(self._samples) < 2:
            return 0
        t0, b0 = self._samples[0]
        t1, b1 = self._samples[-1]
        elapsed = t1 - t0
        if elapsed <= 0:
            return 0
        return (b1 - b0) / elapsed

    def get_eta_seconds(self, total_bytes, downloaded_bytes):
        speed = self.get_speed_bytes_per_sec()
        if speed <= 0:
            return None
        remaining = total_bytes - downloaded_bytes
        if remaining <= 0:
            return 0
        return remaining / speed

    def reset(self):
        self._samples.clear()
        self._total_bytes = 0


class LauncherApp:
    def __init__(self, root):
        self.root = root
        setup_windows_app_id()
        
        # [2026-06-30] FEATURE: Carregar versão do version.json
        self.app_version = self._load_version()
        self.root.title(f"Remanescent Z v{self.app_version}")
        self.root.geometry("640x380")
        self.root.resizable(False, False)
        self.root.configure(bg=BG_COLOR)

        logging.info(f"Inicializando launcher - Python {sys.version}")
        logging.info(f"Diretório de execução: {get_exe_dir()}")
        logging.info(f"Plataforma: {sys.platform}")

        # Corrige ícone na barra de tarefas e no executável
        try:
            icon_path = resource_path("assets/icon.ico")
            if os.path.exists(icon_path):
                self.root.iconbitmap(icon_path)
        except Exception:
            pass

        # Remove barra padrão do Windows e cria barra personalizada
        self.root.overrideredirect(True)
        self._drag_data = {"x": 0, "y": 0}
        self._build_title_bar()

        # Mantém janela borderless mas garante presença na taskbar e minimize correto
        self.root.after(100, lambda: force_taskbar_icon(self.root))
        self.root.bind("<Map>", lambda event: restore_borderless_window(self.root))
        self.root.bind("<Unmap>", lambda e: None)

        # Estado do Launcher
        self.files_to_download = []
        self.total_download_size = 0
        self.downloaded_bytes = 0
        self.current_file_index = 0
        self.is_updating = False
        self.settings_visible = False

        # Throttle para updates de UI
        self._last_ui_update = 0
        self._ui_update_interval = 0.3

        # FASE 2: Fila de updates de UI (substitui lock por arquivo)
        self._verification_queue = queue.Queue()

        # FASE 3: Rastreador de velocidade
        self._speed_tracker = SpeedTracker()

        # FASE 5: Erros de download não-críticos
        self._non_critical_errors = []

        # Carregar diretório de instalação
        self.load_game_directory()

        # FASE 4: Splash screen
        self.show_splash()

        self.setup_ui()

        # Esconde splash e mostra janela principal
        self.root.after(100, self.hide_splash)

        # Inicia verificação em background
        self.trigger_verification()
        
        # [2026-06-30] FEATURE: [6.2.3] Verificar atualização do launcher
        self.root.after(5000, self._notify_launcher_update)

    def _load_version(self):
        """Carrega versão do version.json ou manifest.json."""
        # Try version.json first (generated by CI)
        version_path = os.path.join(get_exe_dir(), "version.json")
        if os.path.exists(version_path):
            try:
                with open(version_path, "r") as f:
                    data = json.load(f)
                    return data.get("version", "dev")
            except Exception:
                pass
        
        # Fallback to manifest.json
        manifest_path = os.path.join(get_exe_dir(), "manifest.json")
        if os.path.exists(manifest_path):
            try:
                with open(manifest_path, "r") as f:
                    data = json.load(f)
                    return data.get("version", "dev")
            except Exception:
                pass
        
        return "dev"

    def _check_launcher_update(self):
        """Verifica se há uma nova versão do launcher disponível no GitHub."""
        try:
            import urllib.request
            import json
            
            # URL do version.json no GitHub
            url = "https://raw.githubusercontent.com/Stoltemberg/dayz-client/main/version.json"
            req = urllib.request.Request(url, headers={"User-Agent": "RMNZ-Launcher"})
            
            with urllib.request.urlopen(req, timeout=10) as response:
                remote_data = json.loads(response.read().decode())
                remote_version = remote_data.get("version", "0")
                local_version = self.app_version
                
                # Comparar versões
                if remote_version != local_version and remote_version != "dev":
                    logging.info(f"Nova versão do launcher disponível: {remote_version} (atual: {local_version})")
                    return remote_version
            
        except Exception as e:
            logging.debug(f"Erro ao verificar atualização do launcher: {e}")
        
        return None

    def _notify_launcher_update(self):
        """Mostra notificação se houver atualização do launcher disponível."""
        try:
            new_version = self._check_launcher_update()
            if new_version:
                from tkinter import messagebox
                result = messagebox.askyesno(
                    "Atualização Disponível",
                    f"Uma nova versão do launcher está disponível: v{new_version}\n\n"
                    f"Versão atual: v{self.app_version}\n\n"
                    f"Deseja atualizar automaticamente?",
                    icon="info"
                )
                if result:
                    self._do_auto_update(new_version)
        except Exception as e:
            logging.debug(f"Erro ao notificar atualização: {e}")

    def _do_auto_update(self, new_version):
        """Baixa e instala a nova versão do launcher."""
        try:
            import urllib.request
            import tempfile
            import shutil
            
            # URL do launcher.exe no GitHub releases
            download_url = f"https://github.com/Stoltemberg/dayz-client/releases/download/v{new_version}/Launcher.exe"
            
            # Mostrar progresso
            from tkinter import messagebox
            messagebox.showinfo(
                "Atualizando",
                f"Baixando launcher v{new_version}...\n\n"
                f"O launcher será reiniciado após a atualização."
            )
            
            # Caminho temporário para download
            temp_dir = tempfile.gettempdir()
            temp_file = os.path.join(temp_dir, f"Launcher_update_{new_version}.exe")
            
            # Baixar novo launcher
            logging.info(f"Baixando launcher v{new_version} de {download_url}")
            urllib.request.urlretrieve(download_url, temp_file)
            
            # Verificar se o download foi bem sucedido
            if os.path.exists(temp_file) and os.path.getsize(temp_file) > 1000:
                # Caminho do launcher atual
                current_exe = sys.executable
                backup_exe = current_exe + ".old"
                
                # Fazer backup do atual
                if os.path.exists(backup_exe):
                    os.remove(backup_exe)
                shutil.copy2(current_exe, backup_exe)
                
                # Substituir pelo novo
                shutil.copy2(temp_file, current_exe)
                
                # Limpar temporário
                os.remove(temp_file)
                
                logging.info(f"Launcher atualizado para v{new_version}. Reiniciando...")
                
                # Reiniciar launcher
                os.execv(sys.executable, [sys.executable] + sys.argv)
            else:
                messagebox.showerror("Erro", "Falha ao baixar atualização. Tente novamente mais tarde.")
                
        except Exception as e:
            logging.error(f"Erro no auto-update: {e}")
            from tkinter import messagebox
            messagebox.showerror("Erro", f"Falha na atualização: {e}")

    def _build_title_bar(self):
        """Cria barra de título personalizada com botões de minimizar e fechar."""
        height = 32
        self.title_bar = tk.Frame(self.root, height=height, bg="#0f0f14", highlightthickness=0)
        self.title_bar.pack(fill="x", side="top")
        self.title_bar.pack_propagate(False)

        self.title_label = tk.Label(
            self.title_bar,
            text=f"Remanescent Z v{self.app_version}",
            font=("Segoe UI", 10, "bold"),
            fg="#e5e5e5",
            bg="#0f0f14",
        )
        self.title_label.pack(side="left", padx=12, pady=4)

        self.btn_minimize = tk.Button(
            self.title_bar,
            text="—",
            width=4,
            height=1,
            bd=0,
            relief="flat",
            bg="#1f1f26",
            fg="#d4d4d4",
            activebackground="#2a2a33",
            activeforeground="#ffffff",
            font=("Segoe UI", 10, "bold"),
            cursor="hand2",
            command=self._minimize_window,
        )
        self.btn_minimize.pack(side="right", padx=(4, 2), pady=4)

        self.btn_close = tk.Button(
            self.title_bar,
            text="✕",
            width=4,
            height=1,
            bd=0,
            relief="flat",
            bg="#7f1d1d",
            fg="#ffffff",
            activebackground="#b91c1c",
            activeforeground="#ffffff",
            font=("Segoe UI", 10, "bold"),
            cursor="hand2",
            command=self._close_window,
        )
        self.btn_close.pack(side="right", padx=(2, 8), pady=4)

        # Arrastar janela pela barra
        for widget in (self.title_bar, self.title_label):
            widget.bind("<Button-1>", self._start_move)
            widget.bind("<B1-Motion>", self._do_move)

    def _start_move(self, event):
        self._drag_data["x"] = event.x
        self._drag_data["y"] = event.y

    def _do_move(self, event):
        x = self.root.winfo_pointerx() - self._drag_data["x"]
        y = self.root.winfo_pointery() - self._drag_data["y"]
        self.root.geometry(f"+{x}+{y}")

    def _minimize_window(self):
        """Minimiza corretamente janela borderless para a barra de tarefas."""
        try:
            minimize_to_taskbar(self.root)
        except Exception:
            pass

    def _close_window(self):
        """Fecha o launcher."""
        try:
            self.root.destroy()
        except Exception:
            pass

    def _on_map_restore_overrideredirect(self, event):
        """Restaura janela sem borda após minimizar/restaurar."""
        try:
            restore_borderless_window(self.root)
        except Exception:
            pass

    def load_game_directory(self):
        """Carrega a configuração de diretório do jogo do launcher_config.json com detecção inteligente."""
        self.game_directory = ""
        if os.path.exists(CONFIG_PATH):
            try:
                with open(CONFIG_PATH, "r", encoding="utf-8") as f:
                    cfg = json.load(f)
                    self.game_directory = cfg.get("game_directory", "")
                    if self.game_directory and not isinstance(self.game_directory, str):
                        logging.warning(f"Configuração de diretório inválida (tipo={type(self.game_directory).__name__})")
                        self.game_directory = ""
            except json.JSONDecodeError as e:
                logging.error(f"Arquivo de configuração corrompido (JSON inválido): {e}")
                self.game_directory = ""
            except Exception as e:
                logging.warning(f"Erro ao ler arquivo de configuração local: {e}")
                self.game_directory = ""

        parent_dir = os.path.abspath(os.path.join(get_exe_dir(), ".."))
        if not self.game_directory or not os.path.exists(os.path.join(self.game_directory, "DayZ.exe")):
            if os.path.exists(os.path.join(get_exe_dir(), "DayZ.exe")):
                self.game_directory = get_exe_dir()
            elif os.path.exists(os.path.join(parent_dir, "DayZ.exe")):
                self.game_directory = parent_dir

        if not self.game_directory or not os.path.exists(os.path.join(self.game_directory, "DayZ.exe")):
            if self.game_directory and self.game_directory.lower().startswith("d:"):
                self.game_directory = r"D:\RMNZ"
            else:
                self.game_directory = r"C:\Program Files\RMNZ"

    # === FASE 4: Splash Screen ===
    def show_splash(self):
        """Mostra splash screen enquanto carrega assets."""
        try:
            self.splash = tk.Toplevel(self.root)
            self.splash.overrideredirect(True)
            self.splash.configure(bg=BG_COLOR)

            w, h = 300, 150
            x = (self.root.winfo_screenwidth() - w) // 2
            y = (self.root.winfo_screenheight() - h) // 2
            self.splash.geometry(f"{w}x{h}+{x}+{y}")

            tk.Label(self.splash, text="REMANESCENT Z",
                     font=("Impact", 20), fg=ACCENT_COLOR, bg=BG_COLOR).pack(pady=(30, 5))
            tk.Label(self.splash, text="Carregando...",
                     font=("Bahnschrift", 9), fg=TEXT_MUTED, bg=BG_COLOR).pack()

            self.splash_bar = tk.Canvas(self.splash, height=3, bg=PROGRESS_BG, highlightthickness=0)
            self.splash_bar.pack(fill=tk.X, padx=40, pady=10)
            self._splash_angle = 0
            self._animate_splash()

            self.root.withdraw()
        except Exception as e:
            logging.warning(f"Erro ao criar splash: {e}")
            self.splash = None

    def _animate_splash(self):
        """Anima a barra do splash (efeito indeterminado)."""
        try:
            if not hasattr(self, 'splash') or not self.splash or not self.splash.winfo_exists():
                return
            self.splash_bar.delete("all")
            w = self.splash_bar.winfo_width()
            if w > 1:
                x = self._splash_angle % (w + 60)
                self.splash_bar.create_rectangle(max(0, x - 60), 0, min(w, x), 3, fill=ACCENT_COLOR, outline="")
            self._splash_angle += 4
            self.root.after(20, self._animate_splash)
        except Exception:
            pass

    def hide_splash(self):
        """Esconde splash e mostra janela principal."""
        try:
            if hasattr(self, 'splash') and self.splash and self.splash.winfo_exists():
                self.splash.destroy()
        except Exception:
            pass
        self.root.deiconify()

    def ensure_write_permission(self):
        """Verifica se o launcher tem permissão de escrita no diretório do jogo."""
        target_dir = os.path.abspath(self.game_directory)

        def check_write(d):
            try:
                if not os.path.exists(d):
                    os.makedirs(d, exist_ok=True)
                test_file = os.path.join(d, ".permission_test")
                with open(test_file, "w") as f:
                    f.write("test")
                os.remove(test_file)
                return True
            except Exception:
                return False

        if check_write(target_dir):
            return True

        # FASE 1: UAC com tratamento robusto de erros
        if not is_admin():
            try:
                wrapper_exe = os.path.abspath(os.path.join(get_exe_dir(), "..", "Launcher.exe"))
                if os.path.exists(wrapper_exe):
                    ret = ctypes.windll.shell32.ShellExecuteW(None, "runas", wrapper_exe, "", None, 1)
                elif getattr(sys, 'frozen', False):
                    ret = ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, subprocess.list2cmdline(sys.argv[1:]), None, 1)
                else:
                    ret = ctypes.windll.shell32.ShellExecuteW(None, "runas", sys.executable, subprocess.list2cmdline(sys.argv), None, 1)

                ret = int(ret)
                if ret > 32:
                    sys.exit(0)
                else:
                    # FASE 1: Mensagens de erro UAC específicas
                    uac_errors = {
                        0: "Erro de memória insuficiente",
                        2: "Arquivo não encontrado",
                        3: "Caminho não encontrado",
                        5: "Acesso negado pelo sistema",
                        8: "Memória insuficiente",
                        266: "Usuário cancelou o UAC",
                        267: "Caminho inválido",
                    }
                    error_msg = uac_errors.get(ret, f"Código de erro: {ret}")
                    logging.error(f"Falha ao elevar privilégios UAC: {error_msg} (código {ret})")
                    messagebox.showerror(
                        "Sem Permissão de Escrita",
                        f"Não foi possível obter permissão de administrador.\n\n"
                        f"Causa: {error_msg}\n\n"
                        f"Execute o launcher como Administrador manualmente.",
                        parent=self.root
                    )
                    self.update_status("Sem Permissão", f"UAC falhou: {error_msg}", 0)
                    self.enable_play_button(False, "SEM PERMISSÃO")
                    return False
            except Exception as e:
                logging.error(f"Exceção durante elevação UAC: {e}")
                messagebox.showerror(
                    "Sem Permissão de Escrita",
                    f"O launcher não possui permissão para escrever na pasta:\n{self.game_directory}\n\n"
                    f"Execute como Administrador ou selecione outro diretório.",
                    parent=self.root
                )
                self.update_status("Sem Permissão", "Execute o launcher como Administrador.", 0)
                self.enable_play_button(False, "SEM PERMISSÃO")
                return False

        messagebox.showerror(
            "Sem Permissão de Escrita",
            f"O launcher não possui permissão para escrever na pasta de instalação:\n{self.game_directory}\n\n"
            f"Execute o launcher como Administrador ou selecione outro diretório nas Configurações.",
            parent=self.root
        )
        self.update_status("Sem Permissão", "Execute o launcher como Administrador.", 0)
        self.enable_play_button(False, "SEM PERMISSÃO")
        return False

    def update_parameter_paths(self):
        """Atualiza dinamicamente os caminhos, DLLs e conexão no !Start_client_parameters.ini."""
        ini_path = os.path.join(self.game_directory, "!Start_client_parameters.ini")
        if not os.path.exists(ini_path):
            logging.warning(f"Arquivo de parâmetros não encontrado: {ini_path}")
            return

        try:
            game_dir = os.path.abspath(self.game_directory)
            if not game_dir.endswith(os.sep):
                game_dir += os.sep

            target_path = os.path.join(game_dir, "DayZ.exe")
            steam_path = os.path.join(game_dir, "SmartSteamEmu", "SmartSteamEmu.dll")
            steam64_path = os.path.join(game_dir, "SmartSteamEmu", "SmartSteamEmu64.dll")

            lines = []
            with open(ini_path, "r", encoding="utf-8-sig") as f:
                lines = f.readlines()

            updated = False
            new_lines = []
            for line in lines:
                stripped = line.strip()
                if stripped.lower().startswith("target"):
                    parts = line.split("=", 1)
                    if len(parts) == 2:
                        line = f"{parts[0]}= {target_path}\n"
                        updated = True
                elif stripped.lower().startswith("startin"):
                    parts = line.split("=", 1)
                    if len(parts) == 2:
                        line = f"{parts[0]}= {game_dir}\n"
                        updated = True
                elif stripped.lower().startswith("steamclientpath"):
                    if stripped.lower().startswith("steamclientpath64"):
                        parts = line.split("=", 1)
                        if len(parts) == 2:
                            line = f"{parts[0]}= {steam64_path}\n"
                            updated = True
                    else:
                        parts = line.split("=", 1)
                        if len(parts) == 2:
                            line = f"{parts[0]}= {steam_path}\n"
                            updated = True
                elif stripped.lower().startswith("commandline"):
                    parts = line.split("=", 1)
                    if len(parts) == 2:
                        cmd_tokens = parts[1].strip().split()
                        replace_prefixes = (
                            "-connect=", "-port=", "-password=", "-name=",
                            "-maxMem=", "-maxVram=", "-cpuCount=",
                        )
                        cmd_tokens = [
                            token for token in cmd_tokens
                            if not any(token.startswith(prefix) for prefix in replace_prefixes)
                            and token not in {"-noPause", "-skipIntro"}
                        ]
                        cmd_tokens.extend([
                            "-noPause", "-skipIntro",
                        ])
                        line = f"{parts[0]} = {' '.join(cmd_tokens)}\n"
                        updated = True
                elif stripped.lower().startswith("broadcastaddress"):
                    parts = line.split("=", 1)
                    if len(parts) == 2:
                        line = f"{parts[0]}= {SERVER_IP}\n"
                        updated = True
                new_lines.append(line)

            if updated:
                with open(ini_path, "w", encoding="utf-8") as f:
                    f.writelines(new_lines)
                logging.info(f"Parâmetros atualizados: {ini_path}")
        except Exception as e:
            logging.error(f"Erro ao atualizar caminhos no arquivo {ini_path}: {e}")

    def save_game_directory(self, new_dir):
        """Salva a nova configuração usando apenas a unidade fixa suportada pelo launcher."""
        try:
            drive = "D" if str(new_dir).lower().startswith("d:") else "C"
            with open(CONFIG_PATH, "w", encoding="utf-8") as f:
                json.dump({"selected_drive": drive}, f, indent=2)
            self.game_directory = r"D:\RMNZ" if drive == "D" else r"C:\Program Files\RMNZ"
            logging.info(f"Unidade do jogo salva: {drive}")
        except Exception as e:
            logging.error(f"Erro ao salvar arquivo de configuração: {e}")

    def reset_settings(self):
        """Restaura configurações para os padrões de fábrica."""
        if messagebox.askyesno(
            "Restaurar Padrões",
            "Tem certeza que deseja restaurar todas as configurações para os valores padrão?\n\n"
            "Isso irá remover o diretório de instalação salvo.",
            parent=self.root
        ):
            try:
                if os.path.exists(CONFIG_PATH):
                    os.remove(CONFIG_PATH)
                self.game_directory = r"C:\Program Files\RMNZ"
                self.drive_var.set("C")
                logging.info("Configurações restauradas para padrão")
                messagebox.showinfo(
                    "Configurações Restauradas",
                    "As configurações foram restauradas para os valores padrão.\n"
                    "Selecione a unidade de instalação e clique em VOLTAR.",
                    parent=self.root
                )
            except Exception as e:
                logging.error(f"Erro ao restaurar configurações: {e}")
                messagebox.showerror(
                    "Erro",
                    f"Não foi possível restaurar as configurações:\n{e}",
                    parent=self.root
                )

    def on_drive_change(self):
        """Atualiza a pasta de instalação para a unidade selecionada."""
        if self.is_updating:
            return
        drive = self.drive_var.get()
        if drive == "C":
            new_dir = r"C:\Program Files\RMNZ"
        else:
            new_dir = r"D:\RMNZ"
        self.save_game_directory(new_dir)
        self.trigger_verification()

    def toggle_settings_view(self):
        """Alterna entre tela principal e tela de configurações."""
        if self.settings_visible:
            self.main_canvas.itemconfigure(self.settings_card_window, state='hidden')
            self.main_canvas.itemconfigure(self.status_card_window, state='normal')
            self.settings_visible = False
        else:
            self.main_canvas.itemconfigure(self.status_card_window, state='hidden')
            self.main_canvas.itemconfigure(self.settings_card_window, state='normal')
            self.settings_visible = True

    def trigger_verification(self):
        """Executa a verificação dos arquivos em segundo plano."""
        if not self.ensure_write_permission():
            return
        threading.Thread(target=self.start_verification, daemon=True).start()

    def setup_ui(self):
        # Canvas Principal
        self.main_canvas = tk.Canvas(
            self.root, width=640, height=348,
            bg=BG_COLOR, highlightthickness=0, bd=0
        )
        self.main_canvas.pack(fill=tk.BOTH, expand=True)

        # Background (FASE 4: só tenta se PIL disponível)
        if HAS_PIL:
            bg_path = resource_path("assets/background.png")
            if os.path.exists(bg_path):
                try:
                    img = Image.open(bg_path)
                    img = img.resize((640, 348), Image.Resampling.LANCZOS)
                    self.bg_img = ImageTk.PhotoImage(img)
                    self.main_canvas.create_image(0, 0, image=self.bg_img, anchor="nw")
                except Exception as e:
                    logging.warning(f"Erro ao carregar background: {e}")

        # Títulos
        self.main_canvas.create_text(
            30, 35, text="REMANESCENT Z",
            font=("Impact", 28), fill=ACCENT_COLOR, anchor="w"
        )
        self.main_canvas.create_text(
            30, 65, text="DayZ Standalone v0.62 - Servidor Privado Local",
            font=("Bahnschrift", 10), fill=TEXT_MUTED, anchor="w"
        )

        # Botão de Configuração
        self.btn_settings = tk.Button(
            self.root, text="\u2699 CONFIGURA\u00c7\u00d5ES",
            font=("Bahnschrift", 9, "bold"),
            bg="#181820", fg=TEXT_MUTED,
            activebackground=ACCENT_COLOR, activeforeground=TEXT_MAIN,
            bd=1, relief="flat", highlightthickness=0,
            padx=12, pady=6, cursor="hand2",
            command=self.toggle_settings_view
        )
        self.main_canvas.create_window(610, 35, window=self.btn_settings, anchor="ne")

        # Card de Status
        self.status_card = tk.Frame(
            self.main_canvas, bg=CARD_COLOR, bd=0,
            highlightbackground=BORDER_COLOR, highlightcolor=BORDER_COLOR, highlightthickness=1
        )

        self.status_title = tk.Label(
            self.status_card, text="Inicializando...",
            font=("Bahnschrift", 11, "bold"), fg=TEXT_MAIN, bg=CARD_COLOR
        )
        self.status_title.pack(anchor="w", padx=18, pady=(12, 4))

        self.status_desc = tk.Label(
            self.status_card, text="Aguardando verificação do manifesto de arquivos.",
            font=("Bahnschrift", 9), fg=TEXT_MUTED, bg=CARD_COLOR,
            wraplength=540, justify="left"
        )
        self.status_desc.pack(anchor="w", padx=18, pady=(0, 10))

        # Barra de progresso customizada (FASE 4: com percentual)
        self.progress_canvas = tk.Canvas(
            self.status_card, height=6,
            bg=PROGRESS_BG, highlightthickness=0, bd=0
        )
        self.progress_canvas.pack(fill=tk.X, padx=18, pady=(0, 12))

        # Footer do card
        self.card_footer = tk.Frame(self.status_card, bg=CARD_COLOR)
        self.card_footer.pack(fill=tk.X, padx=18, pady=(0, 12))

        self.version_label = tk.Label(
            self.card_footer, text="Launcher v2.0.0 | Client 0.62",
            font=("Bahnschrift", 9), fg=TEXT_MUTED, bg=CARD_COLOR
        )
        self.version_label.pack(side=tk.LEFT, anchor="center")

        self.btn_play = tk.Button(
            self.card_footer, text="VERIFICANDO",
            font=("Bahnschrift", 10, "bold"),
            bg="#2c2c35", fg="#777777",
            activebackground="#2c2c35", activeforeground="#777777",
            bd=0, padx=28, pady=6, cursor="arrow",
            state=tk.DISABLED, command=self.on_play_click
        )
        self.btn_play.pack(side=tk.RIGHT)

        # Card de Configurações
        self.settings_card = tk.Frame(
            self.main_canvas, bg=CARD_COLOR, bd=0,
            highlightbackground=BORDER_COLOR, highlightcolor=BORDER_COLOR, highlightthickness=1
        )

        self.settings_title = tk.Label(
            self.settings_card, text="CONFIGURA\u00c7\u00d5ES DO JOGO",
            font=("Bahnschrift", 11, "bold"), fg=ACCENT_COLOR, bg=CARD_COLOR
        )
        self.settings_title.pack(anchor="w", padx=18, pady=(12, 4))

        self.settings_desc = tk.Label(
            self.settings_card, text="Selecione a unidade fixa onde o DayZ está instalado.",
            font=("Bahnschrift", 9), fg=TEXT_MUTED, bg=CARD_COLOR
        )
        self.settings_desc.pack(anchor="w", padx=18, pady=(0, 10))

        # Seção de Drive
        self.dir_section = tk.Frame(self.settings_card, bg=CARD_COLOR)
        self.dir_section.pack(fill=tk.X, padx=18, pady=(0, 10))

        self.lbl_dir_title = tk.Label(
            self.dir_section, text="Unidade de Instalação:",
            font=("Bahnschrift", 9, "bold"), fg=TEXT_MAIN, bg=CARD_COLOR
        )
        self.lbl_dir_title.pack(side=tk.LEFT)

        self.drive_var = tk.StringVar(value="C" if self.game_directory.lower().startswith("c:") else "D")

        self.rbtn_c = tk.Radiobutton(
            self.dir_section, text="Drive C: (C:\\Program Files\\RMNZ)",
            font=("Bahnschrift", 9), variable=self.drive_var, value="C",
            bg=CARD_COLOR, fg=TEXT_MAIN, selectcolor="#181820",
            activebackground=CARD_COLOR, activeforeground=TEXT_MAIN,
            cursor="hand2", command=self.on_drive_change
        )
        self.rbtn_c.pack(side=tk.LEFT, padx=(15, 10))

        self.rbtn_d = tk.Radiobutton(
            self.dir_section, text="Drive D: (D:\\RMNZ)",
            font=("Bahnschrift", 9), variable=self.drive_var, value="D",
            bg=CARD_COLOR, fg=TEXT_MAIN, selectcolor="#181820",
            activebackground=CARD_COLOR, activeforeground=TEXT_MAIN,
            cursor="hand2", command=self.on_drive_change
        )
        self.rbtn_d.pack(side=tk.LEFT)

        # Footer Configurações
        self.settings_footer = tk.Frame(self.settings_card, bg=CARD_COLOR)
        self.settings_footer.pack(fill=tk.X, padx=18, pady=(6, 12))

        self.btn_back = tk.Button(
            self.settings_footer, text="VOLTAR",
            font=("Bahnschrift", 9, "bold"), bg="#24242d", fg=TEXT_MAIN,
            activebackground=ACCENT_COLOR, activeforeground=TEXT_MAIN,
            bd=0, padx=20, pady=5, cursor="hand2",
            command=self.toggle_settings_view
        )
        self.btn_back.pack(side=tk.LEFT)

        self.btn_reset = tk.Button(
            self.settings_footer, text="RESTAURAR PADRÕES",
            font=("Bahnschrift", 9, "bold"), bg="#24242d", fg=TEXT_MUTED,
            activebackground=ACCENT_COLOR, activeforeground=TEXT_MAIN,
            bd=0, padx=12, pady=5, cursor="hand2",
            command=self.reset_settings
        )
        self.btn_reset.pack(side=tk.LEFT, padx=(8, 0))

        self.lbl_future_note = tk.Label(
            self.settings_footer, text="* Configurações salvas automaticamente ao alterar.",
            font=("Bahnschrift", 8, "italic"), fg=TEXT_MUTED, bg=CARD_COLOR
        )
        self.lbl_future_note.pack(side=tk.RIGHT, anchor="center")

        # Inserir cards na canvas
        self.status_card_window = self.main_canvas.create_window(
            320, 230, window=self.status_card, width=580, anchor="center"
        )
        self.settings_card_window = self.main_canvas.create_window(
            320, 230, window=self.settings_card, width=580, anchor="center"
        )
        self.main_canvas.itemconfigure(self.settings_card_window, state='hidden')

        # Garante que a barra de progresso seja desenhada
        self.root.update()
        self.draw_progress(0)

    # === FASE 4: Barra de progresso animada com glow e percentual ===
    def draw_progress(self, percentage):
        """Desenha a barra de progresso com efeito glow e percentual."""
        self.progress_canvas.delete("all")
        width = self.progress_canvas.winfo_width()
        if width <= 1:
            width = 540

        # Fundo
        self.progress_canvas.create_rectangle(0, 0, width, 6, fill=PROGRESS_BG, outline="")

        if percentage > 0:
            fill_width = max(6, int(width * (percentage / 100)))
            # Barra principal
            self.progress_canvas.create_rectangle(0, 0, fill_width, 6, fill=ACCENT_COLOR, outline="")
            # Glow na ponta
            if fill_width > 10:
                self.progress_canvas.create_rectangle(
                    max(0, fill_width - 8), 0, fill_width, 6,
                    fill=ACCENT_HOVER, outline=""
                )

    def update_status(self, title, desc, percentage=0):
        """Atualiza os textos e a barra de progresso com throttle na thread principal."""
        now = time.time()
        if now - self._last_ui_update < self._ui_update_interval:
            return
        self._last_ui_update = now
        self.root.after(0, lambda: self._ui_update(title, desc, percentage))

    def _force_update_status(self, title, desc, percentage=0):
        """Atualiza sem throttle — para estados finais."""
        self.root.after(0, lambda: self._ui_update(title, desc, percentage))

    def _ui_update(self, title, desc, percentage):
        self.status_title.config(text=title)
        self.status_desc.config(text=desc)
        self.draw_progress(percentage)

    def calculate_file_sha256(self, filepath):
        """Calcula SHA256 local — chunk de 1MB (FASE 2)."""
        hash_sha256 = hashlib.sha256()
        try:
            with open(filepath, "rb") as f:
                for chunk in iter(lambda: f.read(1024 * 1024), b""):
                    hash_sha256.update(chunk)
            return hash_sha256.hexdigest()
        except Exception:
            return None

    def calculate_file_md5(self, filepath):
        """Calcula MD5 local — chunk de 1MB (FASE 2)."""
        hash_md5 = hashlib.md5()
        try:
            with open(filepath, "rb") as f:
                for chunk in iter(lambda: f.read(1024 * 1024), b""):
                    hash_md5.update(chunk)
            return hash_md5.hexdigest()
        except Exception:
            return None

    def start_verification(self):
        self.enable_play_button(False, "VERIFICANDO")
        self.update_status("Conectando...", "Buscando manifesto de atualizações remotas no GitHub.")

        # FASE 5: Retry com backoff exponencial no manifesto
        remote_manifest = None
        ssl_context = None
        if HAS_CERTIFI:
            ssl_context = ssl.create_default_context(cafile=certifi.where())
        else:
            ssl_context = ssl.create_default_context()

        for attempt in range(3):
            try:
                url = GITHUB_MANIFEST_API_URL + f"&v={int(time.time())}"
                req = urllib.request.Request(url, headers={
                    'User-Agent': 'Mozilla/5.0',
                    'Accept': 'application/vnd.github+json',
                    'Cache-Control': 'no-cache',
                    'Pragma': 'no-cache',
                })
                timeout = 15 + (attempt * 10)
                with urllib.request.urlopen(req, timeout=timeout, context=ssl_context) as response:
                    manifest_payload = json.loads(response.read().decode('utf-8'))
                if isinstance(manifest_payload, dict) and 'content' in manifest_payload:
                    manifest_json = base64.b64decode(manifest_payload['content']).decode('utf-8')
                    remote_manifest = json.loads(manifest_json)
                else:
                    raise ValueError("Resposta inválida da API do manifesto")
                self._force_update_status("Conectado", "Manifesto de arquivos obtido com sucesso.")
                logging.info(f"Manifesto remoto obtido com sucesso (tentativa {attempt + 1})")
                break
            except Exception as e:
                logging.warning(f"Tentativa {attempt + 1} falhou ao baixar manifesto: {e}")
                if attempt < 2:
                    wait = 2 ** attempt
                    self.update_status("Reconectando...", f"Tentativa {attempt + 2}/3 em {wait}s...")
                    time.sleep(wait)

        if remote_manifest is None:
            logging.warning("Todas as tentativas de manifesto remoto falharam — usando local")
            if os.path.exists(LOCAL_MANIFEST_PATH):
                try:
                    with open(LOCAL_MANIFEST_PATH, "r", encoding="utf-8") as f:
                        remote_manifest = json.load(f)
                    if not isinstance(remote_manifest, dict) or "files" not in remote_manifest:
                        raise ValueError("Manifesto local inválido: estrutura incorreta")
                    self._force_update_status("Modo Local", "Carregado manifesto de integridade local.")
                except Exception as e_local:
                    logging.error(f"Falha ao processar manifesto local: {e_local}")
                    self._force_update_status("Erro", f"Falha ao processar manifesto local: {e_local}")
                    self.enable_play_button(False, "ERRO MANIFESTO")
                    return
            else:
                logging.error("Sem manifesto remoto nem local")
                self._force_update_status(
                    "Offline",
                    "Servidor de atualizações inacessível e sem manifesto local.\n"
                    "Verifique sua conexão e tente novamente."
                )
                self.enable_play_button(False, "SEM CONEXÃO")
                return

        # Carregar Cache Local
        cache = {}
        if os.path.exists(CACHE_PATH):
            try:
                with open(CACHE_PATH, "r", encoding="utf-8") as f:
                    cache_payload = json.load(f)
                    if cache_payload.get("schema_version") == CACHE_SCHEMA_VERSION:
                        cache = cache_payload.get("files", {})
            except Exception:
                pass

        # Comparar Arquivos
        self.update_status("Verificando Integridade...", f"Validando diretório: {self.game_directory}")

        self.files_config = remote_manifest.get("files", {})
        total_files = len(self.files_config)

        self.files_to_download = []
        self.total_download_size = 0

        lock = threading.Lock()
        new_cache = {}
        processed_count = [0]
        self._last_verification_ui_update = 0

        def verify_single_file(relpath, info):
            if should_ignore_manifest_entry(relpath):
                with lock:
                    processed_count[0] += 1
                return

            local_path = os.path.join(self.game_directory, relpath.replace("/", os.sep))
            expected_md5 = info.get("md5")
            expected_size = info.get("size", 0)
            parts_list = info.get("parts", [])

            is_valid = False
            file_size = 0
            file_mtime = 0.0

            if relpath in LOCAL_MUTABLE_FILES and os.path.exists(local_path):
                is_valid = True
            elif os.path.exists(local_path):
                try:
                    file_size = os.path.getsize(local_path)
                    file_mtime = os.path.getmtime(local_path)
                except Exception:
                    pass

                cached_info = cache.get(relpath)
                local_variants = None
                if cached_info and cached_info.get("size") == file_size and cached_info.get("mtime") == file_mtime:
                    local_variants = cached_info.get("variants")

                if not local_variants:
                    local_variants = get_file_integrity_variants(local_path)
                    if local_variants:
                        local_size, local_md5 = local_variants[0]
                        with lock:
                            new_cache[relpath] = {
                                "size": file_size,
                                "mtime": file_mtime,
                                "normalized_size": local_size,
                                "md5": local_md5,
                                "variants": local_variants
                            }
                else:
                    with lock:
                        new_cache[relpath] = cached_info

                if [expected_size, expected_md5] in local_variants or (expected_size, expected_md5) in local_variants:
                    is_valid = True

            with lock:
                processed_count[0] += 1
                now = time.time()
                if now - self._last_verification_ui_update > 0.3 or processed_count[0] == total_files:
                    self._last_verification_ui_update = now
                    progress_pct = int((processed_count[0] / total_files) * 100)
                    self.update_status(
                        "Verificando...",
                        f"Pasta: ...{os.path.basename(self.game_directory)}\nChecando ({processed_count[0]}/{total_files}): {relpath}",
                        progress_pct
                    )

                if not is_valid:
                    if parts_list:
                        part_size = 90 * 1024 * 1024
                        remaining = expected_size
                        for part_relpath in parts_list:
                            this_part_size = min(part_size, remaining)
                            self.files_to_download.append((part_relpath, this_part_size, None))
                            self.total_download_size += this_part_size
                            remaining -= this_part_size
                    else:
                        self.files_to_download.append((relpath, expected_size, expected_md5))
                        self.total_download_size += expected_size

        max_threads = min(8, os.cpu_count() or 4)
        with ThreadPoolExecutor(max_workers=max_threads) as executor:
            futures = {executor.submit(verify_single_file, rp, inf): rp for rp, inf in self.files_config.items()}
            for future in as_completed(futures):
                relpath = futures[future]
                try:
                    future.result()
                except Exception as e:
                    logging.error(f"Erro verificando {relpath}: {e}")

        # Salva o novo cache
        try:
            with open(CACHE_PATH, "w", encoding="utf-8") as f:
                json.dump({"schema_version": CACHE_SCHEMA_VERSION, "files": new_cache}, f, indent=2)
        except Exception as e:
            logging.error(f"Erro ao salvar cache local: {e}")

        # Decidir se precisa atualizar
        if len(self.files_to_download) > 0:
            total_mb = round(self.total_download_size / (1024 * 1024), 2)
            logging.info(f"Atualização necessária: {len(self.files_to_download)} arquivos ({total_mb} MB)")
            self._force_update_status(
                "Atualização Necessária",
                f"Sua instalação diverge da versão oficial. {len(self.files_to_download)} arquivos desatualizados ({total_mb} MB)."
            )
            self.enable_play_button(True, "ATUALIZAR", is_accent=True, is_update=True)
        else:
            logging.info("Todos os arquivos verificados e atualizados")
            self._force_update_status("Jogo Atualizado!", f"Diretório: {self.game_directory}\nTodos os arquivos estão verificados e prontos.")
            self.enable_play_button(True, "JOGAR")

    def enable_play_button(self, enable, text, is_accent=False, is_update=False, retry=False):
        """Habilita ou desabilita o botão Jogar de forma segura na thread principal."""
        self.root.after(0, lambda: self._ui_enable_play(enable, text, is_accent, is_update, retry))

    def _ui_enable_play(self, enable, text, is_accent, is_update, retry=False):
        if enable:
            self.btn_play.config(
                state=tk.NORMAL, text=text,
                bg=ACCENT_COLOR, fg=TEXT_MAIN,
                activebackground=ACCENT_HOVER, activeforeground=TEXT_MAIN,
                cursor="hand2"
            )
            if retry:
                self.btn_play.config(command=self.trigger_verification)
            elif is_update:
                self.btn_play.config(command=self.start_download_flow)
            else:
                self.btn_play.config(command=self.on_play_click)
        else:
            self.btn_play.config(
                state=tk.DISABLED, text=text,
                bg="#2c2c35", fg="#777777",
                activebackground="#2c2c35", activeforeground="#777777",
                cursor="arrow"
            )

    def start_download_flow(self):
        """Inicia o download da fila em segundo plano."""
        if self.is_updating:
            return

        # Validação de espaço em disco
        required_mb = round(self.total_download_size / (1024 * 1024), 1)
        ok, free_mb = check_disk_space(self.game_directory, required_mb + MIN_DISK_SPACE_MB)
        if not ok:
            messagebox.showerror(
                "Espaço Insuficiente",
                f"É necessário pelo menos {required_mb + MIN_DISK_SPACE_MB:.0f} MB livres para atualizar.\n"
                f"Espaço disponível: {free_mb:.0f} MB.\n\n"
                f"Libere espaço e tente novamente.",
                parent=self.root
            )
            self.is_updating = False
            self.enable_play_button(True, "ATUALIZAR", is_accent=True, is_update=True)
            return

        self.is_updating = True
        self._non_critical_errors = []
        self._speed_tracker.reset()
        self.enable_play_button(False, "BAIXANDO...")
        threading.Thread(target=self.download_files_thread, daemon=True).start()

    def _download_single_file(self, relpath, size, expected_md5=None):
        """Baixa um único arquivo do GitHub com suporte real a resume.

        Mantém o parcial em ``.download``. Se houver parcial, tenta HTTP Range.
        Quando o servidor ignora Range e responde 200, reinicia o arquivo com
        segurança para evitar concatenação corrompida.
        Retorna (relpath, True/False, erro).
        """
        local_path = os.path.join(self.game_directory, relpath.replace("/", os.sep))
        os.makedirs(os.path.dirname(local_path), exist_ok=True)
        download_url = GITHUB_RAW_URL + relpath
        temp_path = local_path + ".download"

        ssl_context = None
        if HAS_CERTIFI:
            ssl_context = ssl.create_default_context(cafile=certifi.where())
        else:
            ssl_context = ssl.create_default_context()

        try:
            last_error = None
            for attempt in range(10):
                bytes_written = 0
                resumed_bytes_credited = 0
                try:
                    resume_byte = 0
                    if os.path.exists(temp_path):
                        resume_byte = os.path.getsize(temp_path)
                        # Para arquivos de texto, normaliza tamanho para comparação com manifesto
                        compare_size = resume_byte
                        if is_text_file(temp_path):
                            try:
                                with open(temp_path, "r", encoding="utf-8", errors="ignore") as tf:
                                    compare_size = len(tf.read().replace("\r\n", "\n").encode("utf-8"))
                            except Exception:
                                pass
                        if compare_size == size:
                            os.replace(temp_path, local_path)
                            self.downloaded_bytes += size
                            return (relpath, True, None)
                        if resume_byte > size:
                            os.remove(temp_path)
                            resume_byte = 0

                    headers = {'User-Agent': 'Mozilla/5.0'}
                    if resume_byte > 0:
                        headers['Range'] = f'bytes={resume_byte}-'

                    req = urllib.request.Request(download_url, headers=headers)

                    # Block size adaptativo
                    if size > 500 * 1024 * 1024:
                        block_size = 4 * 1024 * 1024
                    elif size > 100 * 1024 * 1024:
                        block_size = 2 * 1024 * 1024
                    elif size > 10 * 1024 * 1024:
                        block_size = 512 * 1024
                    else:
                        block_size = 262144

                    timeout_secs = max(120, size // (50 * 1024))
                    with urllib.request.urlopen(req, timeout=timeout_secs, context=ssl_context) as response:
                        status = getattr(response, 'status', None) or response.getcode()
                        append_mode = resume_byte > 0 and status == 206

                        if resume_byte > 0 and status != 206:
                            logging.info(f"Servidor ignorou Range para {relpath}; reiniciando download parcial")
                            resume_byte = 0

                        mode = 'ab' if append_mode else 'wb'
                        download_start = time.time()
                        with open(temp_path, mode) as out_file:
                            while True:
                                buffer = response.read(block_size)
                                if not buffer:
                                    break
                                out_file.write(buffer)
                                bytes_written += len(buffer)
                                self.downloaded_bytes += len(buffer)

                                # Throttling de velocidade
                                if DOWNLOAD_SPEED_LIMIT_MBPS > 0:
                                    elapsed = time.time() - download_start
                                    expected_bytes = elapsed * DOWNLOAD_SPEED_LIMIT_MBPS * 1024 * 1024
                                    if bytes_written > expected_bytes:
                                        sleep_time = (bytes_written - expected_bytes) / (DOWNLOAD_SPEED_LIMIT_MBPS * 1024 * 1024)
                                        time.sleep(max(0, sleep_time))

                    actual_size = os.path.getsize(temp_path)
                    # Para arquivos de texto, normaliza CRLF→LF antes de comparar tamanho
                    # (Git armazena LF, GitHub serve LF, mas o manifesto pode ter CRLF)
                    if is_text_file(temp_path):
                        try:
                            with open(temp_path, "r", encoding="utf-8", errors="ignore") as tf:
                                normalized_content = tf.read().replace("\r\n", "\n").encode("utf-8")
                            actual_size = len(normalized_content)
                        except Exception:
                            pass
                    if actual_size != size:
                        raise IOError(f"tamanho inválido após download: esperado {size}, obtido {actual_size}")

                    if expected_md5:
                        # Usa as mesmas variantes de integridade (LF/CRLF) para texto
                        downloaded_variants = get_file_integrity_variants(temp_path)
                        if not any(v[1] == expected_md5 for v in downloaded_variants):
                            actual_md5 = self.calculate_file_md5(temp_path)
                            raise IOError(f"md5 inválido após download: esperado {expected_md5}, obtido {actual_md5}")

                    if resume_byte > 0:
                        resumed_bytes_credited = resume_byte
                        self.downloaded_bytes += resumed_bytes_credited

                    os.replace(temp_path, local_path)
                    return (relpath, True, None)
                except Exception as e_attempt:
                    last_error = str(e_attempt)
                    logging.warning(f"Download {relpath} tentativa {attempt + 1}: {e_attempt}")
                    # HTTP 416 = Range Not Satisfiable: arquivo temp já completo ou inválido
                    if '416' in str(e_attempt):
                        try:
                            if os.path.exists(temp_path):
                                os.remove(temp_path)
                                logging.info(f"Arquivo temp removido após 416: {relpath}")
                        except Exception:
                            pass
                    if bytes_written:
                        self.downloaded_bytes = max(0, self.downloaded_bytes - bytes_written)
                    if resumed_bytes_credited:
                        self.downloaded_bytes = max(0, self.downloaded_bytes - resumed_bytes_credited)
                    if attempt < 9:
                        wait_time = min(30, (2 ** attempt) + (attempt * 0.3))
                        time.sleep(wait_time)

            return (relpath, False, last_error)
        except Exception as e:
            return (relpath, False, str(e))

    def _run_download_batch(self, file_list, max_workers=6):
        """Executa um lote de downloads. Retorna lista de (relpath, size, expected_md5, error) que falharam."""
        completed_count = [0]
        failed_files = []
        total_files = len(file_list)

        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            futures = {
                executor.submit(self._download_single_file, relpath, size, expected_md5): (relpath, size, expected_md5)
                for relpath, size, expected_md5 in file_list
            }

            for future in as_completed(futures):
                relpath, success, error = future.result()
                relpath_key, size_key, md5_key = futures[future]
                if success:
                    completed_count[0] += 1
                else:
                    ext = os.path.splitext(relpath)[1].lower()
                    if ext in NON_CRITICAL_EXTENSIONS:
                        logging.warning(f"Download não-crítico falhou ({relpath}): {error}")
                        self._non_critical_errors.append((relpath, error))
                        self.downloaded_bytes += size_key  # conta como "feito"
                        completed_count[0] += 1
                    else:
                        failed_files.append((relpath_key, size_key, md5_key, error))

        return failed_files

    def download_files_thread(self):
        self.downloaded_bytes = 0
        self._non_critical_errors = []
        total_files = len(self.files_to_download)
        completed_count = [0]
        max_workers = 6
        max_batch_rounds = 5  # retry agressivo em lote

        self._force_update_status(
            f"Baixando 0/{total_files}...",
            f"Iniciando {max_workers} downloads paralelos..."
        )

        # Thread de progresso com velocidade e ETA
        self._progress_running = True
        def progress_updater():
            while self._progress_running:
                if self.total_download_size > 0:
                    self._speed_tracker.update(self.downloaded_bytes)
                    # Cap visual do progresso em 100%
                    capped_bytes = min(self.downloaded_bytes, self.total_download_size)
                    pct = min(100, int((capped_bytes / self.total_download_size) * 100))
                    speed_bytes = self._speed_tracker.get_speed_bytes_per_sec()
                    speed_mb = speed_bytes / (1024 * 1024)
                    total_mb = round(self.total_download_size / (1024 * 1024), 1)
                    downloaded_mb = round(capped_bytes / (1024 * 1024), 1)

                    eta = self._speed_tracker.get_eta_seconds(self.total_download_size, capped_bytes)
                    if eta is not None and eta > 0:
                        eta_m = int(eta // 60)
                        eta_s = int(eta % 60)
                        eta_str = f"{eta_m}m{eta_s:02d}s"
                    else:
                        eta_str = "calculando..."

                    self.update_status(
                        f"Baixando... ({downloaded_mb}/{total_mb} MB)",
                        f"{speed_mb:.1f} MB/s | {pct}% | ETA: {eta_str}",
                        pct
                    )
                time.sleep(0.5)
        progress_thread = threading.Thread(target=progress_updater, daemon=True)
        progress_thread.start()

        # Loop principal: tenta o lote, coleta falhas, retry
        pending = list(self.files_to_download)  # (relpath, size, expected_md5)
        last_error_msg = ""

        for batch_round in range(max_batch_rounds):
            if not pending:
                break

            if batch_round > 0:
                wait_secs = 3 * batch_round
                self._force_update_status(
                    f"Reconectando... ({batch_round}/{max_batch_rounds})",
                    f"Problemas de conexão, tentando novamente em {wait_secs}s..."
                )
                logging.info(f"Retry de lote #{batch_round}: {len(pending)} arquivos, aguardando {wait_secs}s")
                time.sleep(wait_secs)
                self._speed_tracker.reset()

            failed = self._run_download_batch(pending, max_workers)

            if not failed:
                pending = []
                break

            # Coleta erros para mensagem
            last_error_msg = failed[-1][3] or "erro desconhecido"
            pending = [(rp, sz, md5) for rp, sz, md5, _ in failed]
            logging.warning(f"Lote #{batch_round + 1}: {len(failed)} arquivos falharam")

        self._progress_running = False

        # Resumo de erros não-críticos
        if self._non_critical_errors:
            error_summary = ", ".join(p for p, _ in self._non_critical_errors[:5])
            logging.warning(f"Arquivos não-críticos com falha ({len(self._non_critical_errors)}): {error_summary}")

        # Se ainda há pendências após todos os retries
        if pending:
            failed_names = [os.path.basename(rp) for rp, _, _ in pending[:5]]
            failed_str = ", ".join(failed_names)
            if len(pending) > 5:
                failed_str += f" +{len(pending) - 5} mais"

            logging.error(f"Download falhou após {max_batch_rounds} tentativas: {pending[0][0]}")

            if "403" in last_error_msg or "429" in last_error_msg:
                self._force_update_status(
                    "Limite de Tráfego",
                    f"GitHub limitou as requisições. Aguarde 1-2 minutos e clique em TENTAR NOVAMENTE."
                )
            else:
                self._force_update_status(
                    "Conexão Instável",
                    f"{len(pending)} arquivo(s) não baixaram após {max_batch_rounds} tentativas.\nVerifique sua internet e clique em TENTAR NOVAMENTE."
                )
            self.is_updating = False
            self.enable_play_button(True, "TENTAR NOVAMENTE", is_accent=True, retry=True)
            return

        # === FASE PÓS-DOWNLOAD: Reconstrução com progresso ===
        try:
            self._force_update_status("Finalizando...", "Reconstruindo arquivos particionados...", 100)
            logging.info("Iniciando reconstrução de arquivos particionados...")
            self.reconstruct_split_files()
            logging.info("Reconstrução concluída com sucesso")
        except Exception as e:
            logging.error(f"Erro durante reconstrução: {e}")

        self.is_updating = False
        logging.info("Download concluído com sucesso")
        self._force_update_status(
            "Jogo Atualizado!",
            "Download concluído. Todos os arquivos estão na última versão.",
            100
        )
        self.enable_play_button(True, "JOGAR")

    def reconstruct_split_files(self):
        """Varre o manifesto e junta as partes dos arquivos splitados com progresso."""
        if not hasattr(self, 'files_config'):
            return

        split_items = []
        for relpath, info in self.files_config.items():
            parts_list = info.get("parts", [])
            if not parts_list:
                continue
            part_paths = [os.path.join(self.game_directory, p.replace("/", os.sep)) for p in parts_list]
            if any(os.path.exists(p) for p in part_paths):
                split_items.append((relpath, part_paths))

        if not split_items:
            logging.info("Nenhum arquivo particionado para reconstruir")
            return

        total_items = len(split_items)
        for idx, (relpath, part_paths) in enumerate(split_items, 1):
            dest_path = os.path.join(self.game_directory, relpath.replace("/", os.sep))
            os.makedirs(os.path.dirname(dest_path), exist_ok=True)

            self._force_update_status(
                "Reconstruindo...",
                f"Juntando partes ({idx}/{total_items}): {os.path.basename(relpath)}"
            )

            try:
                with open(dest_path, "wb") as dest:
                    for p in part_paths:
                        if os.path.exists(p):
                            with open(p, "rb") as src:
                                while True:
                                    chunk = src.read(4 * 1024 * 1024)
                                    if not chunk:
                                        break
                                    dest.write(chunk)
                            logging.info(f"Parte adicionada: {os.path.basename(p)}")
                        else:
                            logging.warning(f"Parte faltante: {p}")

                actual_size = os.path.getsize(dest_path)
                logging.info(f"Reconstruído: {relpath} ({actual_size} bytes)")

                # Remove partes após reconstrução
                for p in part_paths:
                    if os.path.exists(p):
                        try:
                            os.remove(p)
                        except Exception:
                            pass

            except Exception as e:
                logging.error(f"Erro ao reconstruir {relpath}: {e}")

    def on_play_click(self):
        """Ação ao clicar em JOGAR."""
        self.update_parameter_paths()

        loader_path = os.path.join(self.game_directory, "SmartSteamEmu", "SmartSteamLoader.exe")
        ini_path = os.path.join(self.game_directory, "!Start_client_parameters.ini")

        if not os.path.exists(loader_path):
            logging.error(f"SmartSteamLoader.exe não encontrado: {loader_path}")
            messagebox.showerror(
                "Erro de Inicialização",
                f"O executável do SmartSteamEmu não foi encontrado:\n{loader_path}",
                parent=self.root
            )
            return

        if not os.path.exists(ini_path):
            logging.error(f"Arquivo de parâmetros não encontrado: {ini_path}")
            messagebox.showerror(
                "Erro de Inicialização",
                f"O arquivo de parâmetros do cliente não foi encontrado:\n{ini_path}",
                parent=self.root
            )
            return

        self.update_status("Iniciando Jogo...", f"Conectando a {SERVER_IP}:2302 via Radmin VPN. Bom jogo!")
        logging.info('Iniciando jogo via comando BAT equivalente: start "" "SmartSteamEmu\\SmartSteamLoader.exe" !Start_client_parameters.ini')

        try:
            # Equivalente direto ao .bat:
            # start "" "SmartSteamEmu\SmartSteamLoader.exe" !Start_client_parameters.ini
            creationflags = getattr(subprocess, "CREATE_NO_WINDOW", 0) if os.name == "nt" else 0
            subprocess.Popen(
                [
                    os.path.join(self.game_directory, "SmartSteamEmu", "SmartSteamLoader.exe"),
                    "!Start_client_parameters.ini",
                ],
                cwd=self.game_directory,
                shell=False,
                creationflags=creationflags,
            )
            self.root.after(1500, self.root.destroy)
        except Exception as e:
            logging.error(f"Erro ao iniciar jogo via comando BAT equivalente: {e}")
            messagebox.showerror("Erro ao Jogar", f"Não foi possível iniciar o jogo: {e}", parent=self.root)


if __name__ == "__main__":
    if relaunch_without_console_if_needed():
        sys.exit(0)

    # === FASE 4: DPI Awareness ===
    try:
        ctypes.windll.shcore.SetProcessDpiAwareness(2)
    except Exception:
        try:
            ctypes.windll.user32.SetProcessDPIAware()
        except Exception:
            pass

    root = tk.Tk()
    app = LauncherApp(root)
    root.mainloop()
