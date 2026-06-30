#!/usr/bin/env python3
"""
Bump version based on git commit hash.
Run after generating manifest to embed version info.

Usage: python bump_version.py
Outputs: version.json with commit hash, date, and version string
"""

import subprocess
import json
import os
from datetime import datetime

def get_git_info():
    """Get current git commit hash and date."""
    try:
        # Get short commit hash
        hash_result = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True, text=True, check=True
        )
        commit_hash = hash_result.stdout.strip()
        
        # Get commit date
        date_result = subprocess.run(
            ["git", "log", "-1", "--format=%ci"],
            capture_output=True, text=True, check=True
        )
        commit_date = date_result.stdout.strip()[:10]  # YYYY-MM-DD
        
        # Get commit count (for build number)
        count_result = subprocess.run(
            ["git", "rev-list", "--count", "HEAD"],
            capture_output=True, text=True, check=True
        )
        commit_count = count_result.stdout.strip()
        
        return {
            "hash": commit_hash,
            "date": commit_date,
            "count": commit_count
        }
    except Exception as e:
        print(f"Warning: Could not get git info: {e}")
        return {
            "hash": "unknown",
            "date": datetime.now().strftime("%Y-%m-%d"),
            "count": "0"
        }

def main():
    # Load existing manifest to get base version
    manifest_path = os.path.join(os.path.dirname(__file__), "manifest.json")
    base_version = "0.62.0"
    
    try:
        with open(manifest_path, "r") as f:
            manifest = json.load(f)
            base_version = manifest.get("version", base_version).rsplit(".", 1)[0]
    except Exception:
        pass
    
    # Get git info
    git_info = get_git_info()
    
    # Build version string: BASE.BUILD_COUNT+HASH
    version = f"{base_version}.{git_info['count']}"
    
    # Create version info
    version_info = {
        "version": version,
        "commit": git_info["hash"],
        "date": git_info["date"],
        "build": int(git_info["count"])
    }
    
    # Save version.json
    version_path = os.path.join(os.path.dirname(__file__), "version.json")
    with open(version_path, "w") as f:
        json.dump(version_info, f, indent=2)
    
    # Update manifest version
    try:
        with open(manifest_path, "r") as f:
            manifest = json.load(f)
        manifest["version"] = version
        with open(manifest_path, "w") as f:
            json.dump(manifest, f, indent=2, ensure_ascii=False)
    except Exception as e:
        print(f"Warning: Could not update manifest version: {e}")
    
    print(f"Version: {version}")
    print(f"Commit:  {git_info['hash']}")
    print(f"Date:    {git_info['date']}")
    print(f"Build:   {git_info['count']}")

if __name__ == "__main__":
    main()
