#!/bin/bash

user=$(whoami)
printf "Hello $user! This program will perform setup for PAT software\n"
printf "You may need to enter your computer password to install packages!\n"

printf "Also make sure that this installer file is in a location where you would like to install and store the software!"
printf "These packages will include MySQL, which will require some setup.\nYou can leave the MySQL password blank, but if you choose to fill it in, remember it!\n"
read -p "Press [Enter] when you are ready to continue..."


sudo apt-get -y install mysql-server 
sudo apt-get -y install libmysqlcppconn-dev libmysqlclient-dev openssl libssl-dev libcurl4-openssl-dev libboost-all-dev make g++ git

printf "\n\nDownloading PAT master branch\n\n"

mkdir master
cd master
git init
git remote add origin https://github.com/harrinp/PAT.git
git pull origin master

printf "\n\nChecking that MySQL has started\n"
compare="dead"
status="working"
status=$(service mysql status | grep -o "dead")

if [ -z $status ]; then
	printf "MySQL service is working\n"
else
	printf "It doesn't seem to be working. Trying to start service. This should ask for your computer password!\n\n"
	service mysql start
fi

printf "\nMake sure to test your login using 'mysql -u root (-p {YOURPASSWROD})', including the password if you set one\n"
read -p "Press [Enter] when you are ready to continue..."

printf "\n\nDownloading POCO libraries\n\n"

# This doesn't update itself! Make sure to check for new versions when you come
# back to this!
curl -O https://pocoproject.org/releases/poco-1.9.0/poco-1.9.0-all.tar.gz

tar -zxf poco-1.9.0-all.tar.gz

printf "\n\nCompiling\n\n"

cd poco-1.9.0-all

./configure --no-tests --no-samples
make -j4
sudo make -s install
sudo ldconfig

cd ..
rm -r poco-1.9.0-all
rm poco-1.9.0-all.tar.gz

printf "\n\nInstallation complete!\n"
