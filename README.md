# HSBNE Interlock3

The Interlock3 project is the third iteration of of the ESP8266 firmware for HSBNE's access control system. 

## Development

### Dev Containers
This project uses VS Code dev containers. The dev container is set up to run only on a Linux/WSL2 environment. There are some quality of life features that prevent it working with Windows:
 * The container user is set up with the same GID and UID as the user on the host. This eases issues with file permissions, especially for git.
 * The container mounts your ~/.ssh and ~/.gitconfig directories to make git easier to use.

For best results run `export HOST_GID=$(id -g)` before building the container. The container user will automatically use a GID of 1000 if this is not done.

### Building

In the dev container simply run

```
idf.py build
```

For a fresh build:
```
rm -rf build && idf.py build
```

### Flashing 

To flash the firmware first build the project and then 
```
idf.py -p (PORT) flash
```

## Attributions

The Interlock3 project is made to be compatible with the [MemberMatters](https://github.com/membermatters/MemberMatters) membership portals made, kindly, by Jaimyn Mayer. This project is based in part on the [BeepBeep](https://github.com/membermatters/BeepBeep) firmware by the same author. 

The [original HSBNE access control](https://github.com/HSBNE/AccessControl) project (to the author's knowledge) was created by nog3.