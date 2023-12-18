import os
import platform
import shutil
import sys
import subprocess

def getTag():
    if os.name == "posix":
        try:
            result = subprocess.run(
                ["curl", "-s", "https://api.github.com/repos/TheDevConnor/Zura-Transpiled/releases"],
                capture_output=True,
                text=True,
                check=True
            )

            command = (
                "grep -oP '\"tag_name\":.*?\"pre-release\"' | "
                "grep -oP '\"pre-release\"' | "
                "sed -E 's/\"//g'"
            )

            tag = subprocess.run(
                command,
                input=result.stdout,
                capture_output=True,
                text=True,
                check=True,
                shell=True
            )

            return tag.stdout.strip()
        except subprocess.CalledProcessError as e:
            print(f"Error: {e}")
            return None
    else:
        try:
            result = subprocess.run(
                ["powershell", "(Invoke-RestMethod -Uri 'https://api.github.com/repos/TheDevConnor/Zura-Transpiled/releases').tag_name"],
                capture_output=True,
                text=True,
                check=True
            )

            return result.stdout.strip()
        except subprocess.CalledProcessError as e:
            print(f"Error: {e}")
            return None
        
def installer():
    os_name = platform.system().lower()
    tag = getTag()

    if os_name == "windows":
        url = f"https://github.com/TheDevConnor/Zura-Transpiled/releases/download/{tag}/zura.exe"
        command = ["powershell", "-Command", "Invoke-WebRequest", url]
    elif os_name in ["linux", "darwin"]:
        url = f"https://github.com/TheDevConnor/Zura-Transpiled/releases/download/{tag}/zura"
        command = ["wget", url]
    else:
        return
    
    try:
        subprocess.run(command, stdout=sys.stdout, stderr=sys.stderr, check=True)

        if os_name in ["linux", "darwin"]:
            os.chmod("zura", 0x755)
            command = ["sudo", "mv", "zura", "/usr/local/bin"]
            subprocess.run(command, stdout=sys.stdout, stderr=sys.stderr, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {e}")

installer()