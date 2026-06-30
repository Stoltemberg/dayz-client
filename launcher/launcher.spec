# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['launcher.py'],
    pathex=[],
    binaries=[],
    datas=[
        ('assets/background.png', 'assets'),
        ('assets/icon.ico', 'assets'),
        ('manifest.json', '.'),
    ],
    hiddenimports=[
        'PIL._imaging',
        'PIL.Image',
        'PIL.ImageTk',
        'certifi',
        'email',
        'email.mime.text',
        'email.mime.multipart',
    ],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[
        'test',
        'unittest',
        'numpy',
        'psutil',
        'yaml',
        'setuptools',
        'pip',
        'xmlrpc',
        'pydoc',
        'tkinter.test',
        'tkinter.test.support',
    ],
    noarchive=False,
    optimize=2,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='Launcher',
    debug=False,
    bootloader_ignore_signals=False,
    strip=True,
    upx=True,
    upx_exclude=[],
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon='assets/icon.ico',
    onefile=True,
)