#!/bin/bash

user=$(whoami)
printf "Hello $user! This program will perform setup for PAT software\n"
printf "You may need to enter your computer password to install packages!\n"

printf "Also make sure that this installer file is in a location where you would like to install and store the software!"
printf "These packages will include MySQL, which normally requires some setup.\n Here we are setting the root password to blank, if you would like root to have a password please be sure to set it later.\n"
sleep 2s
read -p "Press [Enter] when you are ready to continue..." placeholder -n 1

sudo apt-get update
# This lets us run the mysql install noninteractively
eport DEBIAN_FRONTEND=noninteractive
# -E uses the environment variable set above, -qq makes things quiet
sudo -E apt-get -qq install mysql-server
sudo apt-get -qq install libmysqlcppconn-dev libmysqlclient-dev openssl libssl-dev libcurl4-openssl-dev libboost-all-dev make g++ git

printf "\n\nDownloading PAT master branch\n\n"
sleep 1s

mkdir master
cd master
git init
git remote add origin https://github.com/harrinp/PAT.git
git pull origin master

printf "\n\nChecking that MySQL has started\n"
sleep 1s
compare="dead"
status="working"
status=$(service mysql status | grep -o "dead")

if [ -z $status ]; then
	printf "MySQL service is working\n"
else
	printf "It doesn't seem to be working. Trying to start service. This should ask for your computer password!\n\n"
	service mysql start
fi
sleep 1s

printf "\nMake sure to test your login using 'mysql -u root (-p {YOURPASSWORD})', including the password if you set one\n"
sleep 2s
read -p "Press [Enter] when you are ready to continue..."

printf "\n\nDownloading POCO libraries\n\n"
sleep 1s

# This doesn't update itself! Make sure to check for new versions when you come
# back to this!
curl -O https://pocoproject.org/releases/poco-1.9.0/poco-1.9.0-all.tar.gz

tar -zxf poco-1.9.0-all.tar.gz

printf "\n\nCompiling\n\n"
sleep 1s

cd poco-1.9.0-all

./configure --no-tests --no-samples
make -j4
sudo make -s install
sudo ldconfig

cd ..
rm -r poco-1.9.0-all
rm poco-1.9.0-all.tar.gz

printf "\n\nBuilding default Config.hpp\n\n"
sleep 1s

cd Headers

printf '''#ifndef CONFIG_HPP
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
#endif''' > Config.hpp


printf "\n\nInstallation complete!\n"
sleep 1s
