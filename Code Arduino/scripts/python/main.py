#!/usr/bin/python
# -*- coding: utf-8 -*-


import csv              # lib pour fichiers csv
import json

from cloudscope import log_error
from cloudscope import log_event
from cloudscope import oauth2_build

DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive '
GMAIL_SCOPE = 'https://mail.google.com/'
SHEET_SCOPE = 'https://spreadsheets.google.com/feeds'

# --------------------------------------------------------------------------------------------------


def get_index():
    """ return the index read in the file indexfile.csv
    :itype : none
    :rtype : integer
    """
    try:
        ifile = open('indexfile.csv', "r")
        data = ifile.read()
        if data == '':
            data = "0"
        ifile.close()
        return int(data)
    except IOError:
        log_error("index file created with value at 0")
        ifile = open('indexfile.csv', "w")
        ifile.write("0")
        ifile.close()

# --------------------------------------------------------------------------------------------------


def put_index(index):
    """ log to the file error.log the current error with the date
    :itype : integer
    :rtype : None
    """
    ifile = open('indexfile.csv', "w")
    ifile.write(str(index))
    ifile.close()
# --------------------------------------------------------------------------------------------------

def worksheet():

    sheet_service = oauth2_build(SHEET_SCOPE)
    wks = sheet_service.open("teleinfo").sheet1
    for ligne in range(1, 10):
        data=float(ligne)*3.48
        wks.update_cell(ligne, 6, data )
        print data
    # val = wks.acell('E1').value
    # print val
    # val = wks.cell(1,2).value
    # print val

# --------------------------------------------------------------------------------------------------
def printrowcvs():

    ifile = open('csvfile.csv', "r")
    index = get_index()
    reader = csv.reader(ifile)
    ligne = 0
    for row in reader:
        if(ligne>=index):
            print 'ligne %s %s' % (ligne, row[0])
        ligne += 1
    put_index(ligne)
    ifile.close()
# --------------------------------------------------------------------------------------------------
def  cvs_to_json():
    """ read a cvs file and write it to a json file
    :itype : integer
    :rtype : None
    """
    csvfile = open('teleinfo.csv', 'r')
    jsonfile = open('file.json', 'w')
    jsonfile.write('[\n')
    fieldnames = ("INDEX_HP","INDEX_HC","MODE","WATER","DATE","TIME")
    reader = csv.DictReader( csvfile, fieldnames)
    for row in reader:
        json.dump(row, jsonfile,indent=4)
        jsonfile.write(',\n')
    jsonfile.write('\b]\n')
# --------------------------------------------------------------------------------------------------

if __name__ == '__main__':
    """ main function
    :itype :
    :rtype : None
    """
    # worksheet()
    cvs_to_json()
    #json_data = open("file.json").read()
    #python_data = json.loads(json_data)
    #print python_data[0]["INDEX_HP"]

