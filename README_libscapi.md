### libScapi

```
$ sudo apt-get update
$ sudo apt-get install -y git build-essential
$ sudo apt-get install -y libssl-ocaml-dev libssl-dev
$ sudo apt-get install -y libgmp3-dev
$ sudo apt-get -y install g++ gcc libicu-dev
$ sudo apt-get install -y liblog4cpp5-dev
```

Download Boost 1.71: (Note that first two steps may not entirely work. If that is the case install boost library manually)

```
$ wget -O boost_1_71_0.tar.bz2 http://sourceforge.net/projects/boost/files/boost/1.71.0/boost_1_71_0.tar.bz2/download
$ tar --bzip2 -xf boost_1_71_0.tar.bz2
$ cd boost_1_71_0
```

Run the script which prepares Boost's build process

```
sudo ./bootstrap.sh --prefix=/usr/local --with-libraries=all
```
 
Compile the project

```
sudo ./b2 install
```
 
Add the Boost libraries path to the default Ubuntu library search path

```
sudo /bin/bash -c 'echo "/usr/local/lib" > /etc/ld.so.conf.d/boost.conf'
```
 
Update the default Ubuntu library search paths

```
sudo ldconfig
```

Clone and make libscapi

```
$ cd ~
$ git clone https://github.com/cryptobiu/libscapi.git
$ cd libscapi
$ make
```

