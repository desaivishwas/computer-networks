"""
Vishwas Desai(visdesai)
Created: 09/09/2021
Client script whcih call netcat instanetcat_e form Netcat.py to relay-
an email from the IU mail server using ESMTP commands
"""
import sys
import os
from netcat import Netcat
def netcat_client():
    """smtp netcat client"""
    netcat_ = Netcat()
    netcat_.connect("mail-relay.iu.edu", 25)
    print("Connection Successful!!")
    print(netcat_.read())
    print("\n")
    # HELO command
    netcat_.write("HELO cricbuzz.com"+"\n")
    print(netcat_.read())
    print("\n")
    # input
    sender = input("MAIL FROM:")
    receiver = input("RCPT TO:")
    subject = input("SUBJECT:")
    file = input("FIELNAME:")
    print("\n")
    # changing directory 
    os.chdir(sys.path[0])
    # reading from a file from a given directory
    with open(f"{file}.txt", 'r') as file:
        data = file.readlines()
        data = ''.join(data)
    # ESMTP Commands
    netcat_.write("MAIL FROM:"+ sender + "\n")
    print(netcat_.read())
    netcat_.write("RCPT TO:"+ receiver + "\n")
    print(netcat_.read())
    netcat_.write("DATA" + "\n")
    netcat_.write("SUBJECT:"+subject + "\n")
    netcat_.write(data + "\n")
    netcat_.write("."+"\n")
    print(netcat_.read())
    print(netcat_.read())
    # netcat_.write("QUIT")
    netcat_.close()

if __name__ == "__main__":
    netcat_client()
