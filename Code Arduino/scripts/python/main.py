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

import csv  # lib pour fichiers csv
import json
import random
import sys
import requests  # Used for the warning InsecurePlatformWarning in python 2.7.3
import warnings  # Used for the warning InsecurePlatformWarning in python 2.7.3
import plotly
import plotly.plotly as py
import plotly.tools as tls
import sqlite3
import calendar
import os.path          # lib pour test fichiers
from plotly.graph_objs import *
from cloudscope import log_error
from cloudscope import log_event
from cloudscope import oauth2_build
from datetime import datetime
from time import gmtime, localtime, strftime

DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive '
GMAIL_SCOPE = 'https://mail.google.com/'
SHEET_SCOPE = 'https://spreadsheets.google.com/feeds'

index_hp = 1451244
index_hc = 1324002
epochdate = 1022214500


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
        data = float(ligne) * 3.48
        wks.update_cell(ligne, 6, data)
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
        if (ligne >= index):
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
    fieldnames = ("INDEX_HP", "INDEX_HC", "MODE", "WATER", "DATE", "TIME")
    reader = csv.DictReader(csvfile, fieldnames)
    for row in reader:
        json.dump(row, jsonfile, indent=4)
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
    trace0 = Scatter(x=[1, 2, 3, 4], y=[50, 15, 23, 17])
    trace1 = Scatter(x=[1, 2, 3, 4], y=[161, 500, 511, 999])

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
    return calendar.timegm(datetime.strptime(datestring, "%Y-%m-%dT%H:%M:%S").timetuple())


def epoch_to_hour(epochtime):
    """
    return the hour from epoch time
    in : Int
    Out Int
    """
    return int(strftime("%H", localtime(epochtime)))

def epoch_to_date(epochtime):
    """
    return the hour from epoch time
    in : Int
    Out Int
    """
    return strftime("%d-%m-%Y", localtime(epochtime))


def epoch_to_year(epochtime):
    """
    return the year from epoch time
    in : Int
    Out Int
    """
    return strftime("%Y", localtime(epochtime))

def epoch_to_month(epochtime):
    """
    return the year from epoch time
    in : Int
    Out Int
    """
    return strftime("%m", localtime(epochtime))

def epoch_to_day(epochtime):
    """
    return the year from epoch time
    in : Int
    Out Int
    """
    return strftime("%d", localtime(epochtime))

def epoch_to_hourminute(epochtime):
    """
    return the hour from epoch time
    in : Int
    Out string
    """
    return strftime("%H:%M", localtime(epochtime))

def epoch_to_minute(epochtime):
    """
    return the minutes from epoch time
    in : Int
    Out Int
    """
    return int(strftime("%M", localtime(epochtime)))


def epoch_to_weekday_name(epochtime):
    """
    return the day of week from epoch time
    ex : 1431959458 return : Monday
    in : Int
    Out String
    """
    wdn = epoch_to_weekday_number(epochtime)

    if wdn==1:
        return 'lundi'
    if wdn==2:
        return 'mardi'
    if wdn==3:
        return 'mercredi'
    if wdn==4:
        return 'jeudi'
    if wdn==5:
        return 'vendredi'
    if wdn==6:
        return 'samedi'
    if wdn==7:
        return 'dimanche'


def epoch_to_week_number(epochtime):
    """
    return the current week (1 ...54)
    ex : 1432203683 return : 21
    in : Int
    Out :Int
    """
    weeknumber = int(datetime.fromtimestamp(epochtime).strftime("%W")) + 1

    return weeknumber


def epoch_to_weekday_number(epochtime):
    """
    return the day of week from epoch time
    1 =
    ex : 1431959458 return : 1
    in : Int
    Out String
    """
    dayt = None
    dayt = int(datetime.fromtimestamp(epochtime).strftime("%w"))
    if (dayt == 0):
        dayt = 7
    return dayt

def teststring():
    """
    For test only this method return a list  like [1256234,1546879,"HP","2012-07-09T22:45:22"]
    the global variable a defined at the beginning of the file
    in : void
    Out list  [Int, Int, String, String]  [1451271, 1324002, 'HP', '2001-12-11T06:21:40']
    """
    global index_hc
    global index_hp
    global epochdate
    epochdate += 3600

    datetime_iso8601 = epoch_to_iso8601(epochdate)
    heures = int(epoch_to_hour(epochdate))
    minutes = int(epoch_to_minute(epochdate))

    mode = "HP"

    if (heures * 60 + minutes > 15 * 60 + 38) and (heures * 60 + minutes < 17 * 60 + 38):  # check between 15h38 and 17h38
        mode = "HC"
    if (heures * 60 + minutes > 21 * 60 + 8) and (heures * 60 + minutes < 23 * 60 + 59):  # check between 21h08 and 23h59
        mode = "HC"
    if (heures * 60 + minutes > 0) and (heures * 60 + minutes < 3 * 60 + 8):  # check between 00h00 and 03h08
        mode = "HC"

    if mode == "HP":
        index_hp += abs(random.randint(1, 100)) # increment HP during HP time
    if mode == "HC":
        index_hc += abs(random.randint(1, 100))  # increment HC during HC time

    liste1 = [index_hp, index_hc, mode, datetime_iso8601]
    liste2 = [index_hp, index_hc, mode]

    return liste1

def testdb_dic_insert(liste):

    nhp = liste[0]
    nhc = liste[1]
    nMode = liste[2]
    nTimestamp = liste[3]
    nEpoch = iso8601_to_epoch(nTimestamp)
    nWday_Name = epoch_to_weekday_name(nEpoch)
    nWday_Number = epoch_to_weekday_number(nEpoch)
    nHour = epoch_to_hourminute(nEpoch)
    nDate = epoch_to_date(nEpoch)
    nWeek = epoch_to_week_number(nEpoch)
    nYear = epoch_to_year(nEpoch)
    nMonth = epoch_to_month(nEpoch)
    nDay = epoch_to_day(nEpoch)

    uTable = 'CurrentWeek'


    try:
        conn = sqlite3.connect('mydatabase.db')

        with conn:
            # connect in dictionary mode
            conn.row_factory = sqlite3.Row
            cur = conn.cursor()

            # count number of row already inserted
            rowsQuery = 'SELECT Count() FROM %s' % uTable
            cur.execute(rowsQuery)
            count = cur.fetchone()[0]

            if count==0:
                # insert the new data
                rowsQuery = 'INSERT INTO %s (Year,Month,Week,WeekDay_Name,Day_Number,Date,Hour,Mode,Index_HP,Index_HC,\
                            Diff_HP,Diff_HC,Diff_HPHC,Cumul_HP,Cumul_HC,Cumul_HPHC)\
                            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)' % uTable
                cur.execute(rowsQuery, (nYear,nWeek, nWday_Name,nWday_Number,nDate,nHour,nMode,nhp,nhc,0,0,0,0,0,0))
            else:

                # select the last inserted data
                rowsQuery = 'SELECT * FROM %s WHERE rowid = %s'% (uTable,count)
                cur.execute(rowsQuery)
                ldata = cur.fetchone()

                # calculate the differencies
                nthp = nhp- ldata["Index_HP"]
                nthc = nhc - ldata["Index_HC"]
                nthphc = nthp + nthc
                nchp = ldata["Cumul_HP"] + nthp
                nchc = ldata["Cumul_HC"] + nthc
                nchphc = nchp + nchc

                # insert the new data
                rowsQuery = 'INSERT INTO %s (Year,Week,WeekDay_Name,Day_Number,Date,Hour,Mode,Index_HP,Index_HC,\
                            Diff_HP,Diff_HC,Diff_HPHC,Cumul_HP,Cumul_HC,Cumul_HPHC)\
                            VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)' % uTable
                cur.execute(rowsQuery, (nYear,nWeek, nWday_Name,nWday_Number,nDate,nHour,nMode,nhp,nhc,nthp,nthc,nthphc,nchp,nchc,nchphc))

    except sqlite3.Error, e:
        print "Error %s:" % e.args[0]

def testdb_insert(liste):

    nhp = liste[0]
    nhc = liste[1]
    nMode = liste[2]
    nTimestamp = liste[3]
    nEpoch = iso8601_to_epoch(nTimestamp)
    nDay_Name = epoch_to_weekday_name(nEpoch)
    nDate = epoch_to_date(nEpoch)
    nDay_Number = epoch_to_weekday_number(nEpoch)
    nHour = epoch_to_hourminute(nEpoch)
    nTable = 'CurrentWeek'

    try:
        conn = sqlite3.connect('mydatabase.db')

        with conn:

            cur = conn.cursor()
            rowsQuery = 'SELECT Count() FROM %s' % nTable
            cur.execute(rowsQuery)
            count = cur.fetchone()[0]
            print " last rowid is %s" % count

            if count != 0:
                rowsQuery = 'SELECT * FROM Week WHERE rowid = %s'% count
                cur.execute(rowsQuery)
                pdata = cur.fetchone()
                php = pdata[3]
                phc = pdata[4]
                print "Data Tuple  from rowid %s = %s" % (count, pdata)


            rowsQuery = 'INSERT INTO %s (Year,Week) VALUES(?,?)' % nTable
            cur.execute(rowsQuery, (2015, 41))
            lid = cur.lastrowid
            print " new rowid is %s" % lid

    except sqlite3.Error, e:
        print "Error %s:" % e.args[0]


def create_database():
    """
    Create a sqlite3 database
    in : void
    Out : void
    """
    if os.path.isfile('mydatabase.db'):
        os.remove('mydatabase.db')

    conn = sqlite3.connect('mydatabase.db')
    # create a table
    with conn:

        cur = conn.cursor()   Year,

        # create a table for the week
        # create a table for the detailed Days
        cur.execute('CREATE TABLE CurrentWeek( Year INTEGER,Year Integer,Week INTEGER,WeekDay_Number INTEGER,\
                    Day INTEGER, Day_Name TEXT,Date TEXT, Hour TEXT, \
                    Mode TEXT, Index_HP INTEGER, Index_HC INTEGER, \
                    Diff_HP INTEGER, Diff_HC INTEGER, Diff_HPHC INTEGER, \
                    Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER, \
                    Dummy1 INTEGER, Dummy2 INTEGER );')

        cur.execute('CREATE TABLE Week(Year INTEGER, Week INTEGER, \
                    Index_HP INTEGER, Index_HC INTEGER, \
                    Diff_HP INTEGER, Diff_HC INTEGER, Diff_HPHC INTEGER, \
                    Dummy1 INTEGER, Dummy2 INTEGER);')

        # create a table for the Month
        cur.execute('CREATE TABLE Month(Year INTEGER, Month INTEGER, Month_Name TEXT,\
                    Index_HP INTEGER, Index_HC INTEGER, \
                    Diff_HP INTEGER, Diff_HC INTEGER, Diff_HPHC INTEGER, \
                    Dummy1 INTEGER, Dummy2 INTEGER);')

        # create a table for the Days
        cur.execute('CREATE TABLE Year(Year INTEGER, Month INTEGER, Day INTEGER, \
                    Index_HP INTEGER, Index_HC INTEGER, \
                    Diff_HP INTEGER, Diff_HC INTEGER, Diff_HPHC INTEGER, \
                    Dummy1 INTEGER, Dummy2 INTEGER);')



if __name__ == '__main__':

    #print epoch_to_weekday_number(1432117359)
    #print epoch_to_weekday_name(1432117359)
    create_database()
    #testdb_insert('Week',1023,118)
    for ligne in range(1, 50):
        nliste = teststring()
        testdb_dic_insert(nliste)