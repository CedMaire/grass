# grass
Implementation of GRASS (GRep AS a Service) for CS-412

# Team
- Benjamin Délèze, benjamin.deleze@epfl.ch, GitHub: BenjaminDeleze
- Cedric Maire, cedric.maire@epfl.ch, GitHub: CedMaire
- Mathilde Raynal, mathilde.raynal@epfl.ch, GitHub: PizzaWhisperer

# Usage
Once you are in the grass folder, run `make` to build the executables.
The configuration file `grass.conf` should be structured as follow :
```
base [base_dir]

port [port_number]

user [user1] [pass1]
user [user2] [pass2]
# ...
user [userN] [passN]
```
[`grass.conf`](https://github.com/CedMaire/grass/blob/master/grass.conf) can be used as example.

## Start the server
```bash
./bin/server
```
uses the configuration file to set up the users' folders (each user is working on their own directory and has access to their files only) and launches the server.

## Start the client
The client can be started in two different modes, manual or automatic. In both modes, the server ip address and the port number specified in the configuration file have to be entered as arguments.
```bash
./bin/client [server ip] [port]
```
launches the client in manual mode, meaning it prints a prompt ">>>" and the client has to enter by hand each command and receives feedback from the server on the same terminal. 
```bash
./bin/client [server ip] [port] [input file] [output file]
```
launches the client in automatic mode. Instead of reading the user input from a shell, it reads the commands written on the file `[input file]` and writes the server's feedback on `[output file]`.

## Functions
The client has access to the following set of functions :

 - `login [user]` The login command starts authentication. The username must be one of the allowed usernames in the configuration file.
 - `pass [password]` The pass command must directly follow the login command. The password must match the password for the earlier specified user. If the password matches, the user is successfully authenticated.
 - `ping [host]` The server will respond with the output of the Unix command ping $HOST -c 1. 
 - **`ls`** The ls command lists the available files in the current working directory in the format as reported by ls -l. 
 - **`cd [dir name]`** The cd command changes the current working directory to the specified one.
 - **`mkdir [dir name]`** The mkdir command creates a new directory with the specified name in the current working direc- tory.
 - **`rm [name]`** The rm command deletes the file or directory with the specified name in the current working directory.
 - **`get [filename]`** The get command retrieves a file from the current working directory.
 - **`put [filename] [size]`** The put command sends the specified file from the current local working directory to the server.
 
*Notes: the commands get and put can be executed in parallel. Also, it may take some time before the new files are shown when running the `ls` command.*
 - **`date`** The date command returns the output from the Unix date command.
 - **`grep [regex]`** The grep command searches every file in the current directory and its subdirectory for the requested pattern. The pattern follows the Regular Expressions [rules](https://www.gnu.org/software/grep/manual/html_node/Fundamental-Structure.html).
 - **`whoami`** The whoami command returns the name of the currently logged in user.
 - **`w`** The w command returns a list of each logged in user on a single line space separated.
 - **`logout`** The logout command logs the user out of their session.
 - `exit`
 
A bold command means that this command may only be executed after a successful authentication.

Some exploits were intentionally put in the programs. Please refer to [exploits_README.md](https://github.com/CedMaire/grass/blob/master/Exploits_README.md) to learn more about them.
Password for the zip-file: `_gXPR@ek&ukj!B5n_T!-fU7KKFz#A9u&Raqe!sCxdFHf?K#GYJP=xKQK2TjQvmYZ`
