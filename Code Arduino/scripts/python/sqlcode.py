#!/usr/bin/python
# -*- coding: utf-8 -*-

import sqlite3
import os.path  # lib pour test fichiers
from cloudscope import log_error
from datetime import datetime
from timefunc import TimeFunc

# DATABASE_NAME = '/root/python/database.db'
# DATABASE_NAME = 'database.db'


class SqlBase:

    def __init__(self):
        pass

    @staticmethod
    def create(dbname):
        """
        create the database
        """
        if not os.path.isfile(dbname):
            try:
                conn = sqlite3.connect(dbname)
                # create a table
                with conn:
                    cur = conn.cursor()

                    # create a table for the detailed Days
                    cur.execute('CREATE TABLE CurrentWeek( Year INTEGER, Month INTEGER, Day INTEGER,\
                                Week_Number INTEGER, WeekDay_Number INTEGER,\
                                Day_Name TEXT, Date TEXT, Hour TEXT, \
                                Mode TEXT, Index_HP INTEGER, Index_HC INTEGER, \
                                Diff_HP INTEGER, Diff_HC INTEGER, Diff_HPHC INTEGER, \
                                Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER, Temp_In REAL, Temp_Out REAL, UPLOADED INTEGER);')
                    # create a table for the Days
                    cur.execute('CREATE TABLE Day(Year INTEGER, Month INTEGER, Day INTEGER, Day_Name TEXT, Week_Number INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER,Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                Temp_In_Min REAL, Temp_In_Avg REAL, Temp_In_Max REAL, \
                                Temp_Out_Min REAL, Temp_Out_Avg REAL, Temp_Out_Max REAL, UPLOADED INTEGER);')
                    # create a table for the week
                    cur.execute('CREATE TABLE Week(Year INTEGER, Week_Number INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER,\
                                Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER, \
                                Temp_In_Min REAL, Temp_In_Avg REAL, Temp_In_Max REAL, \
                                Temp_Out_Min REAL, Temp_Out_Avg REAL, Temp_Out_Max REAL, UPLOADED INTEGER);')
                    # create a table for the Month
                    cur.execute('CREATE TABLE Month(Year INTEGER, Month INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER, Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                Temp_In_Min REAL, Temp_In_Avg REAL, Temp_In_Max REAL,\
                                Temp_Out_Min REAL, Temp_Out_Avg REAL, Temp_Out_Max REAL, UPLOADED INTEGER);')
                    # create a table for the Year
                    cur.execute('CREATE TABLE Year(Year INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER, Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                Temp_In_Min REAL, Temp_In_Avg REAL, Temp_In_Max REAL,\
                                Temp_Out_Min REAL, Temp_Out_Avg REAL, Temp_Out_Max REAL, UPLOADED INTEGER);')
                    # create a table for events
                    cur.execute('CREATE TABLE Event(Plotly INTEGER, Day_Counter INTEGER);')

            except sqlite3.Error, e:
                log_error("Error when try to create database in create_database() ")
                log_error(str(e))
                print str(e)
                exit()
                # print "Error %s:" % e.args[0]

    @staticmethod
    def update(dbname, test, dataliste):
        """
        Store new incoming data into the database
        :param dbname, mode, dataliste
        :return:  None
        """
        max_days = 1

        if test == 1:  # TEST MODE

            nhp = dataliste[0]
            nhc = dataliste[1]
            nmode = dataliste[2]
            ntimestamp = dataliste[3]
            tin = 99
            tout = 99
            nepoch = TimeFunc.iso8601_to_epoch(ntimestamp)
            ndayna = TimeFunc.epoch_to_weekday_name(nepoch)
            nwdaynu = TimeFunc.epoch_to_weekday_number(nepoch)
            nhour = TimeFunc.epoch_to_hourminute(nepoch)
            ndate = TimeFunc.epoch_to_date(nepoch)
            nweekn = TimeFunc.epoch_to_week_number(nepoch)
            nyear = TimeFunc.epoch_to_year(nepoch)
            nmonth = TimeFunc.epoch_to_month(nepoch)
            nday = TimeFunc.epoch_to_day(nepoch)

        else:

            nhp = int(dataliste[0])                         # HP index from arduino
            nhc = int(dataliste[1])                         # HC index from arduino
            nmode = str(dataliste[2])                       # HC or HP mode from arduino
            tin = float(dataliste[3])                       # Temperature sensor in
            tout = 99                                       # no sensor at the moment
            ndayna = datetime.now().strftime("%A")          # day name string
            nwdaynu = int(datetime.now().strftime("%w"))    # weekday number decimal
            nhour = datetime.now().strftime("%H:%M")        # Hour:Minute string
            ndate = datetime.now().strftime("%d-%m-%Y")     # day decimal
            nweekn = int(datetime.now().strftime("%W"))     # week number
            nyear = int(datetime.now().strftime("%Y"))      # week number
            nmonth = int(datetime.now().strftime("%m"))     # month decimal
            nday = int(datetime.now().strftime("%d"))       # day decimal

        SqlBase.create(dbname)

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode

                conn.row_factory = sqlite3.Row
                cur = conn.cursor()

                # count number of row already inserted in table currentweek **************************************
                cur.execute('SELECT Count() FROM CurrentWeek')
                count = cur.fetchone()[0]

                if count == 0:  # init case

                    sqlquery = 'INSERT INTO CurrentWeek (Year, Month, Day, Week_Number, WeekDay_Number, Day_Name, Date,Hour,\
                                Mode, Index_HP, Index_HC, Diff_HP, Diff_HC, Diff_HPHC , \
                                Cumul_HP, Cumul_HC, Cumul_HPHC, Temp_In, Temp_Out, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nday, nweekn, nwdaynu, ndayna, ndate,
                                           nhour, nmode, nhp, nhc, 0, 0, 0, 0, 0, 0, tin, tout, 0))

                    sqlquery = 'INSERT INTO Week (Year, Week_Number,\
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, Temp_In_Min, Temp_In_Avg, Temp_In_Max,\
                                Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nweekn, nhp, nhc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    sqlquery = 'INSERT INTO Month (Year, Month,\
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, Temp_In_Min, Temp_In_Avg, Temp_In_Max,\
                                Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nhp, nhc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    sqlquery = 'INSERT INTO Year (Year, Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                                Temp_In_Min, Temp_In_Avg, Temp_In_Max, Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nhp, nhc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    sqlquery = 'INSERT INTO Day (Year, Month, Day, Day_Name, Week_Number, \
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, Temp_In_Min, Temp_In_Avg, Temp_In_Max,\
                                Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nday, ndayna, nweekn, nhp, nhc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    sqlquery = 'INSERT INTO Event (Plotly, Day_Counter) VALUES(?,?)'
                    cur.execute(sqlquery, (0, 0))

                else:

                    # ############################### CurrentWeek Table PROCESSING    ###########################################

                    sqlquery = 'INSERT INTO CurrentWeek (Year,Month,Day,Week_Number,WeekDay_Number,Day_Name,Date,Hour,\
                            Mode, Index_HP, Index_HC, Diff_HP, Diff_HC, Diff_HPHC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                            Temp_In, Temp_Out, UPLOADED) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'

                    cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['WeekDay_Number'] == nwdaynu:
                        # calculate the differencies if it is the same day
                        ndhp = nhp - previous_data['Index_HP']
                        ndhc = nhc - previous_data['Index_HC']
                        ndhphc = ndhp + ndhc
                        nchp = previous_data['Cumul_HP'] + ndhp
                        nchc = previous_data['Cumul_HC'] + ndhc
                        nchphc = nchp + nchc
                        cur.execute(sqlquery, (nyear, nmonth, nday, nweekn, nwdaynu, ndayna, ndate, nhour,
                                               nmode, nhp, nhc, ndhp, ndhc, ndhphc, nchp, nchc, nchphc, tin, tout, 0))

                    else:
                        # new day
                        ndhp = nhp - previous_data['Index_HP']
                        ndhc = nhc - previous_data['Index_HC']
                        ndhphc = ndhp + ndhc
                        nchp = ndhp
                        nchc = ndhc
                        nchphc = nchp + nchc

                        cur.execute(sqlquery, (nyear, nmonth, nday, nweekn, nwdaynu, ndayna, ndate,
                                               nhour, nmode, nhp, nhc, ndhp, ndhc, ndhphc, nchp, nchc, nchphc, tin, tout, 0))

                        # Erase table if we have recorded 2 days max
                        # print "Current Day = %d" % previous_data['Day']
                        cur.execute('SELECT * FROM Event WHERE rowid = %d' % 1)
                        previous_data = cur.fetchone()
                        day_counter = previous_data['Day_Counter']
                        if day_counter < max_days:
                            day_counter += 1
                            cur.execute('UPDATE  Event SET Day_Counter = %d WHERE rowid = 1' % day_counter)
                        else:
                            cur.execute('UPDATE  Event SET Plotly = 1 WHERE rowid = 1')
                            cur.execute('SELECT * FROM CurrentWeek WHERE rowid = 1')
                            previous_data = cur.fetchone()['Day']
                            cur.execute('DELETE FROM CurrentWeek WHERE Day = %d' % previous_data)
                            # print "Day %d removed" % previous_data
                            cur.execute('VACUUM')  # #### VERY IMPORTANT ##### #

                    # ###############################  Day Table PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Day')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Day WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['Day'] == nday:
                        # same day
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Day SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Day SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Day SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Day SET UPLOADED = %d WHERE rowid = %d' % (0, count))
                        # Calculate Min AVG Max of temperature In---------------------------------------
                        cur.execute('SELECT MIN(Temp_In) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_In_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_In) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_In_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_In) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_In_Max = %f WHERE rowid = %d' % (ntemp, count))
                        # Calculate Min AVG Max of temperature Out--------------------------------------
                        cur.execute('SELECT MIN(Temp_Out) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_Out_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_Out) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_Out_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_Out) FROM CurrentWeek where Day = %d' % nday)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Day SET Temp_Out_Max = %f WHERE rowid = %d' % (ntemp, count))

                    else:
                        # new Day
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Day (Year, Month, Day, Day_Name, Week_Number,\
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, Temp_In_Min, Temp_In_Avg,\
                                    Temp_In_Max, Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nmonth, nday, ndayna, nweekn, nchp, nchc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    # ###############################  Week Table PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Week')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Week WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['Week_Number'] == nweekn:
                        # same week
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Week SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Week SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Week SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Week SET UPLOADED = %d WHERE rowid = %d' % (0, count))

                        # Calculate Min AVG Max of temperature In---------------------------------------
                        cur.execute('SELECT MIN(Temp_In_Min) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_In_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_In_Avg) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_In_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_In_Max) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_In_Max = %f WHERE rowid = %d' % (ntemp, count))

                        # Calculate Min AVG Max of temperature Out--------------------------------------
                        cur.execute('SELECT MIN(Temp_Out_Min) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_Out_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_Out_Avg) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_Out_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_Out_Max) FROM Day where Week_Number = %d And Year = %d' % (nweekn, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Week SET Temp_Out_Max = %f WHERE rowid = %d' % (ntemp, count))

                    else:
                        # new week
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Week (Year, Week_Number, Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                                    Temp_In_Min, Temp_In_Avg, Temp_In_Max, Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nweekn, nchp, nchc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                     # ###############################  Month Table PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Month')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Month WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()
                    if previous_data['Month'] == nmonth:
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Month SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Month SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Month SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Month SET UPLOADED = %d WHERE rowid = %d' % (0, count))
                        # Calculate Min AVG Max of temperature In---------------------------------------
                        cur.execute('SELECT MIN(Temp_In_Min) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_In_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_In_Avg) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_In_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_In_Max) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_In_Max = %f WHERE rowid = %d' % (ntemp, count))

                        # Calculate Min AVG Max of temperature Out--------------------------------------
                        cur.execute('SELECT MIN(Temp_Out_Min) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_Out_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_Out_Avg) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_Out_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_Out_Max) FROM Day where Month = %d And Year = %d' % (nmonth, nyear))
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Month SET Temp_Out_Max = %f WHERE rowid = %d' % (ntemp, count))

                    else:
                        # new month
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Month (Year, Month,\
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                                    Temp_In_Min, Temp_In_Avg, Temp_In_Max, Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nmonth, nchp, nchc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

                    # ###############################  Year Table PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Year')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Year WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['Year'] == nyear:
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Year SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Year SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Year SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Year SET UPLOADED = %d WHERE rowid = %d' % (0, count))
                        # Calculate Min AVG Max of temperature In---------------------------------------
                        cur.execute('SELECT MIN(Temp_In_Min) FROM Day where  Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_In_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_In_Avg) FROM Day where Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_In_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_In_Max) FROM Day where Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_In_Max = %f WHERE rowid = %d' % (ntemp, count))

                        # Calculate Min AVG Max of temperature Out--------------------------------------
                        cur.execute('SELECT MIN(Temp_Out_Min) FROM Day where Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_Out_Min = %f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT AVG(Temp_Out_Avg) FROM Day where Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_Out_Avg = %.1f WHERE rowid = %d' % (ntemp, count))

                        cur.execute('SELECT MAX(Temp_Out_Max) FROM Day where Year = %d' % nyear)
                        ntemp = cur.fetchone()[0]
                        cur.execute('UPDATE Year SET Temp_Out_Max = %f WHERE rowid = %d' % (ntemp, count))
                    else:
                        # new year
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Year (Year,\
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                                    Temp_In_Min, Temp_In_Avg, Temp_In_Max, Temp_Out_Min, Temp_Out_Avg, Temp_Out_Max, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nchp, nchc, 0, 0, 0, tin, tin, tin, tout, tout, tout, 0))

        except sqlite3.Error, e:
            log_error("Error database access in database_update()")
            log_error(str(e))
            print str(e)
            exit()


    @staticmethod
    def resetflagupload(dbname):

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()

                # Search the first rowid with uploaded = 0   ****************
                cur.execute('SELECT Count() FROM CurrentWeek')
                count = cur.fetchone()[0]
                for x in range(1, count+1):
                    cur.execute('UPDATE  CurrentWeek SET UPLOADED = %d WHERE rowid = %d' % (0, x))

                cur.execute('SELECT Count() FROM Day')
                count = cur.fetchone()[0]
                for x in range(1, count+1):
                    cur.execute('UPDATE  Day SET UPLOADED = %d WHERE rowid = %d' % (0, x))

                cur.execute('SELECT Count() FROM Week')
                count = cur.fetchone()[0]
                for x in range(1, count+1):
                    cur.execute('UPDATE  Week SET UPLOADED = %d WHERE rowid = %d' % (0, x))

                cur.execute('SELECT Count() FROM Month')
                count = cur.fetchone()[0]
                for x in range(1, count+1):
                    cur.execute('UPDATE  Month SET UPLOADED = %d WHERE rowid = %d' % (0, x))

                cur.execute('SELECT Count() FROM Year')
                count = cur.fetchone()[0]
                for x in range(1, count+1):
                    cur.execute('UPDATE  Year SET UPLOADED = %d WHERE rowid = %d' % (0, x))

        except sqlite3.Error, e:
            log_error("Error database access in resetflagupload()")
            log_error(str(e))
            print str(e)
            exit()


    @staticmethod
    def minvalue(dbname):

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()

                # Search the first rowid with uploaded = 0
                cur.execute('SELECT min(Temp_In) FROM CurrentWeek where Day = 26')
                valeur = cur.fetchone()[0]
                print valeur
                cur.execute('SELECT max(Temp_In) FROM CurrentWeek where Day = 26')
                valeur = cur.fetchone()[0]
                print valeur
                cur.execute('SELECT avg(Temp_In) FROM CurrentWeek where Day = 26')
                valeur = cur.fetchone()[0]
                print valeur
        except sqlite3.Error, e:
            print e
            exit()