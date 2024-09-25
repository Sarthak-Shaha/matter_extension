import subprocess

# List all submodules
result = subprocess.run(['git', 'submodule', 'status'], capture_output=True, text=True)
submodules = result.stdout.splitlines()

# Filter out submodules that are installed by other means in Studio
excluded_submodules = {'simplicity_sdk', 'wifi_sdk', 'wiseconnect-wifi-bt-sdk', 'matter_private'}
filtered_submodules = [line.split()[1] for line in submodules if line.split()[1] not in excluded_submodules]

# Initialize and update the remaining submodules
for submodule in filtered_submodules:
    subprocess.run(['git', 'submodule', 'update', '--init', submodule])
