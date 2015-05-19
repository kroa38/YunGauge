#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
utilise un fichier Json du format suivant pour la config
{
      "source_mail":"email",
      "dest_mail":"email",
      "folder_id":"the_working_folder id_on_google_drive",
      "arduino_config" : {
                          "sampling_interval":5,
                          "adjust_rtc":20
                          }

}

"""

import csv              # lib pour fichiers csv
import json
import sys
import requests			# Used for the warning InsecurePlatformWarning in python 2.7.3
import warnings		    # Used for the warning InsecurePlatformWarning in python 2.7.3
import plotly
import plotly.plotly as py
import plotly.tools as tls
import sqlite3 as lite
import calendar
from plotly.graph_objs import *
from cloudscope import log_error
from cloudscope import log_event
from cloudscope import oauth2_build
from datetime import datetime
from time import gmtime, localtime,  strftime

DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive '
GMAIL_SCOPE = 'https://mail.google.com/'
SHEET_SCOPE = 'https://spreadsheets.google.com/feeds'

index_hp = 1451244
index_hc = 1324002
epochdate = 1008034500

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


def put_index(index):
    """ log to the file error.log the current error with the date
    :itype : integer
    :rtype : None
    """
    ifile = open('indexfile.csv', "w")
    ifile.write(str(index))
    ifile.close()


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


def cvs_to_json():
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


def plotly_test():
    """ Plotly test
    before using you must run he command below. This command create a credential json file
    in your $HOME/.plotly directory.
    plotly.tools.set_credentials_file(username='username', api_key='key', stream_ids=['id1', 'id2'])
    :itype : none
    :rtype : None
    """
    # Used for ignore the warning InsecurePlatformWarning in python 2.7.3
    requests.packages.urllib3.disable_warnings()

    credentials = tls.get_credentials_file()
    trace0 = Scatter(x=[1, 2, 3, 4],y=[50, 15, 23, 17])
    trace1 = Scatter(x=[1, 2, 3, 4],y=[161, 500, 511, 999])

    data = Data([trace0, trace1])

    unique_url = py.plot(data, filename='basic-line')


def epoch_to_iso8601(epochtime):
    """
    convert the unix epoch time into a iso8601 formatted date
    epoch_to_iso8601(1341866722) return   '2012-07-09T22:45:22'
    In : Int
    Out : String
    """
    return datetime.fromtimestamp(epochtime).isoformat()


def iso8601_to_epoch(datestring):
    """
    iso8601_to_epoch - convert the iso8601 date into the unix epoch time
    In : String
    Out :
    """
    return calendar.timegm(datetime.strptime(datestring, "%Y-%m-%dT%H:%M:%S.%f").timetuple())

def epoch_to_hour(epochtime):
    """
    return the hour from epoch time
    in : Int
    Out Int
    """
    return int(strftime("%H", localtime(epochtime)))


def epoch_to_minute(epochtime):
    """
    return the minutes from epoch time
    in : Int
    Out Int
    """
    return int(strftime("%M", localtime(epochtime)))


def epoch_to_day(epochtime):
    """
    return the day of week from epoch time
    ex : 1431959458 return : Monday
    in : Int
    Out String
    """
    return datetime.fromtimestamp(epochtime).strftime("%A")


def teststring():
    """
    For test only this method return a list  like [1256234,1546879,"HP","2012-07-09T22:45:22"]
    the global variable a defined at the beginning of the file
    in : void
    Out list  [Int, Int, String, String]
    """
    global index_hc
    global index_hp
    global epochdate
    epochdate += 900   # 900 = 15 minutes = 15*60

    heureminute = epoch_to_iso8601(epochdate)
    heures = int(epoch_to_hour(epochdate))
    minutes = int(epoch_to_minute(epochdate))

    mode = "HP"

    if (heures*60+minutes > 15*60+38) and (heures*60+minutes < 17*60+38):  # check between 15h38 and 17h38
        mode = "HC"
    if (heures*60+minutes > 21*60+8) and (heures*60+minutes < 23*60+59):   # check between 21h08 and 23h59
        mode = "HC"
    if (heures*60+minutes > 0) and (heures*60+minutes < 3*60+8):           # check between 00h00 and 03h08
        mode = "HC"

    if mode == "HP":
        index_hp += 1       # increment HP during HP time
    if mode == "HC":
        index_hc += 1       # increment HC during HC time

    liste1 = [index_hp, index_hc, mode, heureminute]

    return liste1


def testdb():

    con = None

    try:
        con = lite.connect('test.db')

        cur = con.cursor()
        cur.execute('SELECT SQLITE_VERSION()')

        data = cur.fetchone()

        print "SQLite version: %s" % data

    except lite.Error, e:

        print "Error %s:" % e.args[0]
        sys.exit(1)

    finally:

        if con:
            con.close()


if __name__ == '__main__':

    print(iso8601_to_epoch("1008034500"))