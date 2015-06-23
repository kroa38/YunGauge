#!/usr/bin/python
# -*- coding: utf-8 -*-

import sqlite3
import os.path, sys  # lib pour test fichiers
from cloudscope import log_error
from datetime import datetime
from timefunc import TimeFunc

# DATABASE_NAME = '/root/python/database.db'
# DATABASE_NAME = 'database.db'


class DataBase:

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
                                Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER, UPLOADED INTEGER);')
                    # create a table for the Days
                    cur.execute('CREATE TABLE Day(Year INTEGER, Month INTEGER, Day INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER,Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                UPLOADED INTEGER);')
                    # create a table for the week
                    cur.execute('CREATE TABLE Week(Year INTEGER, Week_Number INTEGER, \
                                 Index_HP INTEGER, Index_HC INTEGER,\
                                 Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER, UPLOADED INTEGER);')
                    # create a table for the Month
                    cur.execute('CREATE TABLE Month(Year INTEGER, Month INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER, Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                UPLOADED INTEGER);')
                    # create a table for the Year
                    cur.execute('CREATE TABLE Year(Year INTEGER, \
                                Index_HP INTEGER, Index_HC INTEGER, Cumul_HP INTEGER, Cumul_HC INTEGER, Cumul_HPHC INTEGER,\
                                UPLOADED INTEGER);')
                    # create a table for events
                    cur.execute('CREATE TABLE Event(Plotly INTEGER, Day_Counter INTEGER);')

            except sqlite3.Error:
                log_error("Error when try to create database in create_database() ")
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
            ndayna = datetime.now().strftime("%A")          # day name string
            nwdaynu = int(datetime.now().strftime("%w"))    # weekday number decimal
            nhour = datetime.now().strftime("%H:%M")        # Hour:Minute string
            ndate = datetime.now().strftime("%d-%m-%Y")     # day decimal
            nweekn = int(datetime.now().strftime("%W"))     # week number
            nyear = int(datetime.now().strftime("%Y"))      # week number
            nmonth = int(datetime.now().strftime("%m"))     # month decimal
            nday = int(datetime.now().strftime("%d"))       # day decimal

        maxweek = 1

        DataBase.create(dbname)

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                event_clean = 0

                conn.row_factory = sqlite3.Row
                cur = conn.cursor()

                # count number of row already inserted in table currentweek **************************************
                cur.execute('SELECT Count() FROM CurrentWeek')
                count = cur.fetchone()[0]

                if count == 0:  # init case

                    sqlquery = 'INSERT INTO CurrentWeek (Year, Month, Day, Week_Number, WeekDay_Number, Day_Name, Date,Hour,\
                                Mode, Index_HP, Index_HC, Diff_HP, Diff_HC, Diff_HPHC , \
                                Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nday, nweekn, nwdaynu, ndayna, ndate,
                                           nhour, nmode, nhp, nhc, 0, 0, 0, 0, 0, 0, 0))

                    sqlquery = 'INSERT INTO Week (Year, Week_Number,\
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nweekn, nhp, nhc, 0, 0, 0, 0))

                    sqlquery = 'INSERT INTO Month (Year, Month,\
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nhp, nhc, 0, 0, 0, 0))

                    sqlquery = 'INSERT INTO Year (Year, \
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nhp, nhc, 0, 0, 0, 0))

                    sqlquery = 'INSERT INTO Day (Year, Month, Day, \
                                Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                VALUES(?,?,?,?,?,?,?,?,?)'
                    cur.execute(sqlquery, (nyear, nmonth, nday, nhp, nhc, 0, 0, 0, 0))

                    sqlquery = 'INSERT INTO Event (Plotly, Day_Counter) VALUES(?,?)'
                    cur.execute(sqlquery, (0, 0))

                else:

                    # ############################### CurrentWeek PROCESSING    ###########################################

                    sqlquery = 'INSERT INTO CurrentWeek (Year,Month,Day,Week_Number,WeekDay_Number,Day_Name,Date,Hour,\
                            Mode, Index_HP, Index_HC, Diff_HP, Diff_HC, Diff_HPHC, Cumul_HP, Cumul_HC, Cumul_HPHC,\
                            UPLOADED) VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)'

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
                                               nmode, nhp, nhc, ndhp, ndhc, ndhphc, nchp, nchc, nchphc, 0))

                    else:
                        # new day
                        ndhp = nhp - previous_data['Index_HP']
                        ndhc = nhc - previous_data['Index_HC']
                        ndhphc = ndhp + ndhc
                        nchp = ndhp
                        nchc = ndhc
                        nchphc = nchp + nchc

                        cur.execute(sqlquery, (nyear, nmonth, nday, nweekn, nwdaynu, ndayna, ndate,
                                               nhour, nmode, nhp, nhc, ndhp, ndhc, ndhphc, nchp, nchc, nchphc, 0))

                        #Erase table if we have recorded 2 days max
                        print "Current Day = %d" % previous_data['Day']
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
                            print "Day %d removed" % previous_data
                            cur.execute('VACUUM')  # #### VERY IMPORTANT ##### #

                    # ###############################  Day PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Day')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Day WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['Day'] == nday:
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Day SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Day SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Day SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Day SET UPLOADED = %d WHERE rowid = %d' % (0, count))
                    else:
                        # new Day
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Day (Year, Month, Day, \
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nmonth, nday, nchp, nchc, 0, 0, 0, 0))

                    # ###############################  Week PROCESSING    ##############################################
                    cur.execute('SELECT Count() FROM Week')
                    count = cur.fetchone()[0]
                    cur.execute('SELECT * FROM Week WHERE rowid = %d' % count)
                    previous_data = cur.fetchone()

                    if previous_data['Week_Number'] == nweekn:
                        nchp = nhp - previous_data['Index_HP']
                        nchc = nhc - previous_data['Index_HC']
                        nchphc = nchp + nchc
                        cur.execute('UPDATE  Week SET CUMUL_HP = %d WHERE rowid = %d' % (nchp, count))
                        cur.execute('UPDATE  Week SET CUMUL_HC = %d WHERE rowid = %d' % (nchc, count))
                        cur.execute('UPDATE  Week SET CUMUL_HPHC = %d WHERE rowid = %d' % (nchphc, count))
                        cur.execute('UPDATE  Week SET UPLOADED = %d WHERE rowid = %d' % (0, count))
                    else:
                        # new week
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Week (Year, Week_Number,\
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nweekn, nchp, nchc, 0, 0, 0, 0))

                        # ----------------------------------------------------------------------------
                        #     Remove Week in the CurrentWeek table
                        # ----------------------------------------------------------------------------
                        '''if count > maxweek:
                            cur.execute('SELECT * FROM Week WHERE rowid = %d' % count)
                            last_week = cur.fetchone()['Week_Number'] - maxweek
                            cur.execute('DELETE FROM CurrentWeek WHERE Week_Number = %d' % last_week)
                            cur.execute('VACUUM')  # #### VERY IMPORTANT ##### #
                            event_clean = 1'''

                    # ###############################  Month PROCESSING    ##############################################
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
                    else:
                        # new month
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Month (Year, Month,\
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nmonth, nchp, nchc, 0, 0, 0, 0))

                    # ###############################  Year PROCESSING    ##############################################
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
                    else:
                        # new year
                        nchp = previous_data['Index_HP'] + previous_data['Cumul_HP']
                        nchc = previous_data['Index_HC'] + previous_data['Cumul_HC']
                        sqlquery = 'INSERT INTO Year (Year, \
                                    Index_HP, Index_HC, Cumul_HP, Cumul_HC, Cumul_HPHC, UPLOADED)\
                                    VALUES(?,?,?,?,?,?,?)'
                        cur.execute(sqlquery, (nyear, nchp, nchc, 0, 0, 0, 0))

            return event_clean

        except sqlite3.Error:
            log_error("Error database access in database_update()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def resetflagupload(dbname):

        conn = sqlite3.connect(dbname)

        with conn:
            # connect database in dictionary mode
            conn.row_factory = sqlite3.Row
            cur = conn.cursor()

            # Search the first rowid with uploaded = 0   ****************
            cur.execute('SELECT Count() FROM %d' % 'CurrentWeek')
            count = cur.fetchone()[0]
            for x in range(1, count+1):
                cur.execute('UPDATE  CurrentWeek SET UPLOADED = %d WHERE rowid = %d' % (0, x))

            cur.execute('SELECT Count() FROM %d' % 'Day')
            count = cur.fetchone()[0]
            for x in range(1, count+1):
                cur.execute('UPDATE  Day SET UPLOADED = %d WHERE rowid = %d' % (0, x))

            cur.execute('SELECT Count() FROM %d' % 'Week')
            count = cur.fetchone()[0]
            for x in range(1, count+1):
                cur.execute('UPDATE  Week SET UPLOADED = %d WHERE rowid = %d' % (0, x))

            cur.execute('SELECT Count() FROM %d' % 'Month')
            count = cur.fetchone()[0]
            for x in range(1, count+1):
                cur.execute('UPDATE  Month SET UPLOADED = %d WHERE rowid = %d' % (0, x))

            cur.execute('SELECT Count() FROM %d' % 'Year')
            count = cur.fetchone()[0]
            for x in range(1, count+1):
                cur.execute('UPDATE  Year SET UPLOADED = %d WHERE rowid = %d' % (0, x))