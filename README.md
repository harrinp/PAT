# PAT
Open-source trading application developed and maintained by the Purdue Algorithmic Trading Club

These directions are tested to work on Ubuntu 16.04 and some other Debian based distros, as well as possibly the Windows Linux subsystem (See end of the installation instructions for special information about Linux Subsystem). The directions use apt, though if you prefer apt-get then nothing is lost. Users on Windows are encouraged to use the Linux subsystem. For Mac users, substitute brew commands for any package installation.

## Installation

### Installer **(EXPERIMENTAL)**
We have been working on an installer that should currently function for Debian based Linux systems. Keep in mind that this installer has only been tested on a Linux Mint 18.3 system, and may not work on other systems. If you would like to try the installer instead of the manual install process, do the following:

1. Create a folder for you to store your PAT files, somewhere you can find it and with a descriptive name.
2. Open a terminal window in the folder you created.
3. Run `curl "https://raw.githubusercontent.com/harrinp/PAT/master/installer.sh" > installer.sh && chmod +x ./installer.sh && ./installer.sh && rm installer.sh`. This will download the installer and immediately run it with the appropriate shell, removing it when it is finished.
4. Keep in mind that the installer will ask for your password, as it needs to install packages on your system. The password prompts come entirely from package management tools like apt-get.
5. Check the steps below for MySQL, setting up the config file, and compiling PAT, as well as any troubleshooting steps you may need to do for your system.

Please let us know if you had any problems with the installer! We are trying to improve it and feedback is a key part of that process.

### Github Setup

1. Make a Github account if you intend to contribute code.
2. Make sure you have a directory to put the code in. Within that folder, make a folder for the master branch of the code.
3. If you do not have git, install it with `sudo apt install git`
4. Open a terminal window in the folder for the master branch that you created earlier, and type `git init`
5. `git remote add origin https://github.com/harrinp/PAT.git`
6. `git pull origin master`
7. Wait for the process to complete, and check out the folders for your shiny new algorithmic trading code.

### MySQL Setup

1. Run `sudo apt install mysql-server libmysqlcppconn-dev libmysqlclient-dev` This should install the client, the server and the libraries to interface with C++.
2. At some point you will go through a set up process for the MySQL server. Set the password for root to whatever you like, and remember it. The current recommendation is to leave the root password blank for easier interaction with the code, however if you intend to use MySQL for other purposes you may want a password.
3. Make sure that MySQL is started with `service mysql status`
4. If it’s not started run `service mysql start`
5. Try to login with `mysql -u root (-p {YOURPASSWORD})` Only do the password related items in parentheses if you set a password.
6. If you can’t connect, try adding sudo to the command.
7. If you can connect when you use sudo then you will have trouble getting the code to connect. Try these directions, adding your password if you set one:

![alt text](https://github.com/harrinp/PAT/blob/master/Images/readme1.png "Fixing messed up MySQL permissions")

To complete step 2 in those directions you will want to run `select host, user, password from mysql.user;`

8. If you can log in now, you’re good.
9. While you are logged in, run the command "CREATE database quotesDB;" This makes the database for the example code that we’ll run to make sure everything works.
10. Also run the following commands to get your database setup for the Moving Average Convergence Divergence (MACD) indicator:
``` SQL
CREATE TABLE `MACD_EUR_USD_D` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
```
``` SQL
CREATE TABLE `MACD_EUR_USD_M1` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
```
``` SQL
CREATE TABLE `MACD_EUR_USD_H1` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
```

### Compiling and Installing POCO

1. This library has to be compiled from source to do what we need. Head to their website and download the latest COMPLETE version of the linux source. 1.9.0 is confirmed working, though the newest version should also work.
2. Make sure you install some packages that some of POCO depends on, or it won’t compile everything. These include `openssl` `libssl-dev` `libcurl4-openssl-dev` and `libboost-all-dev`
3. For the rest of these directions, `make` is used rather than `gmake`. `make` is confirmed to work for this process, however if you prefer `gmake` it should also work.
4. From there, check that you have a version of make that is at least 3.80 or newer with `make --version`
5. Go to where you downloaded the library and do these commands BUT with some changes:

![alt text](https://github.com/harrinp/PAT/blob/master/Images/readme2.png "Compiling POCO")

6. For `./configure` instead run `./configure --no-tests --no-samples` This reduces build times by skipping some steps that are not typically necessary.
7. For `make -s` instead run `make -j4` This reduces build times by using 4 workers to do the compilation.
8. The make might take a long time. Be patient.
9. Follow this with `sudo make -s install` to get the files into the appropriate directories.
10. That should work, but if you keep getting compile errors from not having some libraries, try running `sudo ldconfig` which should create any missing symbolic links. Additional troubleshooting steps for Linux Subsystem users are at the end of the installation instructions, as the subsystem can be a bit tricky for libraries.

### Config file
1. Navigate to the Headers folder in your installation.
2. Create a file called `Config.hpp` and paste the following into it:
``` C++
#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <unistd.h>
// MYSQL parameters
static const std::string URL = "tcp://127.0.0.1:3306";
static const std::string USER = "root";
static const std::string PASSWORD = "";
static const std::string DATABASE_NAME = "quotesDB";
// OANDA parameters
static const std::string ACCOUNT_ID = "";
static const std::string ACCESS_TOKEN = "";
#endif
```
3. Fill in the `USER` and `PASSWORD` fields with the user and password for your MySQL user (default is `root` and blank password). Also, to run the program against an account you will need generate an access token from an OANDA account and fill out the `ACCESS_TOKEN` and `ACCOUNT_ID` fields for your account. Change the `DATABASE_NAME` field if you decided you wanted to have a different database name during the MySQL setup.

### Compiling PAT from Source

1. Open a terminal window where you downloaded the code
2. To make sure things work, run "make example"
3. If it fails to compile because of libraries, check to make sure you installed everything above correctly. If you did, try "make exampleU" (This activates the makes for Debian based systems such as Ubuntu, which can help).
4. One you can get all the recipes (EXCEPT executor that is only for testing) to compile, try to run the example with "./runE.out"
5. If you get rejected by MySQL, make sure that you have entered your password for your MySQL user

From here, you should make an OANDA demo account. This will let you generate an API key, and then be able to put that in the program and start tinkering.

## Linux Subsystem

### Subsystem Installation

If you are on windows, you should be able to still do most of these directions through the Windows Linux subsystem for Windows 10. To enable it, do the following:

1. Windows search for Turn Windows features on or off
2. Enable the Windows Subsystem for Linux
3. Reboot
4. Open Settings > Update & Security > for developers and enable developer mode
5. Windows search for bash
6. Download the Ubuntu component of the Linux Subsystem from the Windows Store

### POCO Installation and Libraries

The Windows subsystem is not very good at having libraries installed to it, so this takes a bit more work than on other systems. Here are some modified POCO install steps:

1. Once you download the complete POCO source, navigate to your downloads using `cd "/mnt/c/Users/%YOURUSERNAME%/Downloads/"`, and then copy the downloaded files to your home directory using `cp poco-X.X.X-all.tar.gz ~` where X.X.X is replaced with your version number.
2. Follow the standard POCO installation instructions above
3. Run the following commands from the folder where you built POCO
```
sudo cp -r Foundation/include/Poco/. /usr/include/Poco/
sudo cp -r Util/include/Poco/. /usr/include/Poco/
sudo cp -r JSON/include/Poco/. /usr/include/Poco/
sudo cp -r lib/Linux/x86_64/. /usr/local/lib
```
This moves the POCO files to a reasonable library location.

4. To make sure that your system will look for the libraries in the right place, start by opening `~/.bashrc`
5. Add the line `LD_LIBRARY_PATH=/usr/local/lib` at the end of the file. Save and restart your terminal.
6. Run `sudo ldconfig` to build the symbolic links necessary for GCC to find everything.

## Usage Tips

Once you can get the `make example`, `make analyze`, and `make decide` commands to run (or their U suffix counterparts), you're ready to start using the system. Run the example script using `./runE.out` in your installation folder. This should begin downloading some of the historical data. It is recommended to let the data download overnight, since it can take a long time even with a fast drive. It is okay to stop the example code once started, but *do not* run the example code again. It will start over from the beginning and waste time. Instead, run the analysis code.

The first time you run the analysis code, use the command `./runA.out -i`. This signals the program to complete initialization of the tables. Any other time you want to run the analysis code, run the command `./runA.out`. This program will begin by updating the historical price data to the most current it can find on the OANDA servers. It will then begin to conduct a Moving Average Convergence Divergence (MACD) analysis. Both the download and the MACD calculations can take awhile, so you may want to let this program run overnight. Once it reaches the present day in the MACD analysis, it will periodically check for new data, download it, and analyze it.

The decision making code can be run using the command `./runD.out`. We are currently working on changing a lot about how this code functions, so if it doesn't work, don't worry. Look for future updates to address functionality of this component. Additionally, the make for the executor program will fail if you try to run it. This is currently the expected behavior because of changes being made to the code.
