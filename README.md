# Oblivious Transfer

## Installation instruction:

- In your home directory type:

```
$ git clone https://github.com/manel1874/DDHOT.git
```

- Install libscapi using dev branch: https://github.com/cryptobiu/libscapi/tree/dev. In case it does not compile, please follow the instructions in README_libscapi.md.

- Copy the elements inside Final\_version\_working to libscapi folder:

```
$ cd

$ cp -r DDHOT/Final_version_working libscapi
```

- Update your current home directory in the makefile file. (ex. change manel to your username)

- Run:

```
$ cd ~/libsacpi

$ make
```

## Run example

- Run in separate terminals:

Terminal 1
```
$ ./runOT 0 0
```

Terminal 2
```
./runOT 1 1
```

- The first input corresponds to the party type (0: Sender; 1: Receiver);
- For a Sender Party the second input corresponds to the crs setup type (0: Messy mode; 1: Decryption mode);
- For a Receiver Party the second input corresponds to the chosen element, sigma (0: m0, 1: m1).









