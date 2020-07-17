# Sapphire Language (Ad hoc implementation)

## Intro
(Not ready)

## Build Binary
### Windows Platform
Compile them in Visual Studio 2017 or later versions.(MSVC 14+).
Just add all source files into a new solution. You can also build them with cl.exe manually.

Notice: Mingw/Cygwin is NOT OFFICIAL SUPPORTED for now. You can post issues for these resolutions but I may not fix 
them until I start working on them.

### Linux/BSD Platform
gcc 8.0+ or clang 8.0+ are recommended, and you also need cmake/make to finish configurations.
MacOS is NOT OFFICIAL SUPPORTED for now and I don't have Apple device for testing.

```
# Archlinux/Manjaro Linux
sudo pacman -S cmake make gcc

# clone & build
git clone https://github.com/RinGotou/Sapphire-Adhoc.git sapphire-adhoc
cd sapphire-adhoc
git submodule init && git submodule update
cmake .
make
```
## Help me?
You can post issues or create pull request. 

## License
BSD-2-Clause
