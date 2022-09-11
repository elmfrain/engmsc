import json
import os

_tools_config_file = open("tools_config.json", "r")
tools_config = json.load(_tools_config_file)
_tools_config_file.close()

_readme_file = open(tools_config["readme_file"], "r")
readme = _readme_file.read()
_readme_file.close()

def need_help(args):
    for arg in args:
        if(arg == "-h" or arg == "--help"):
            return True

    return False

def get_tool_name(file):
    file = os.path.basename(file)

    tool_name = "unknown"

    try:
        tool_name = tools_config["scripts"][file]["name"]
    except KeyError:
        print("Script \"%s\" is not known in tools_config.json" % (file))

    return tool_name

def get_tool_help(file):
    file = os.path.basename(file)

    tool_help = "not available"

    copy = False
    for line in readme.splitlines():
        if line.startswith("Script: %s" % (file)):
            tool_help = ""
            copy = True
        elif line.startswith("EndScript") and copy:
            break
        if copy:
            tool_help += line + "\n"

    return tool_help

def get_tool_options(file):
    file = os.path.basename(file)

    tool_parameters = None

    try:
        tool_parameters = tools_config["scripts"][file]["options"]
    except KeyError:
        print("Script \"%s\" appears to not have any options stored in tools_config;json" % (file))

    return tool_parameters

def main():
    print("This is a core for the tools. Run other scripts instead:\n")
    print("Add the flag -h or --help to recieve help information for that script\n")

    scripts = tools_config["scripts"]

    for script in scripts:
        if script == "tools_core.py":
            continue
        tool_name = tools_config["scripts"][script]["name"]
        tool_summary = tools_config["scripts"][script]["summary"]
        print("[%s] %s: %s" % (script, tool_name, tool_summary) + "\n")

if __name__ == "__main__":
    main()