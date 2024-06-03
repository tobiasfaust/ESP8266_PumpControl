Import("env");
import sys, os, re;
from shutil import copytree;

if (re.match(r".*ESP32.*", env["PIOENV"])):
    print("prepareDataDir.py: ESP32 detected");
    esptype = "esp32"
elif (re.match(r".*ESP8266.*", env["PIOENV"])):
    print("prepareDataDir.py: ESP8266 detected");
    esptype = "esp8266"
else:
    print("dont match any esp");
    exit;

if (esptype):
    data_master_dir = "esp_files";
    data_dir = "data/web/esp";

    if (os.path.exists(data_master_dir +"/"+ esptype)):
        copytree(data_master_dir +"/"+ esptype + "/" , data_dir, dirs_exist_ok=True);
        print("copy :<" + data_master_dir +"/"+ esptype + "> to <" + data_dir + ">");
    else:
        print("path not exists: " + data_master_dir +"/"+ esptype + "/");