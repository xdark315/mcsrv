# MCSRV


Run and stop minecraft serveur for linux


**Installalation**  
`make `  
`make install `  

**Usage**

`mcsrv status `  -> get server status

`mcsrv start `   -> run the server

`mcsrv stop `    -> stop the server

`mcsrv restart ` -> stop the server


**Config**

config file in /home/user

config file example:
``` json
{
    "srv": "example name",
    "folder": "/home/user/srv_folder",
    "run": "run.sh"
}
```