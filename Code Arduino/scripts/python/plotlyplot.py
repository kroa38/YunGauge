#!/usr/bin/python
# -*- coding: utf-8 -*-

import sqlite3
import plotly.plotly as py
import plotly.tools as tls
import requests  # Used for the warning InsecurePlatformWarning in python 2.7.3
from plotly import exceptions
from plotly.graph_objs import *
from cloudscope import log_error

# DATABASE_NAME = '/root/python/database.db'
# DATABASE_NAME = 'database.db'


class PlotlyPlot:

    def __init__(self):
        pass

    @staticmethod
    def plot(dbname, event_clean):
        """
        Send to plotly the data
        :param event_clean from data_base
        :return:   none
        """
        try:
            # for unlock websense (quiet and delete after download)
            '''os.system("wget -q --delete-after www.google.fr")
            _ = urllib2.urlopen('http://www.google.fr/', timeout=4)'''

            PlotlyPlot.currentweek(dbname, event_clean)
            PlotlyPlot.day(dbname)
            PlotlyPlot.week(dbname)
            PlotlyPlot.month(dbname)
            PlotlyPlot.year(dbname)

        except urllib2.URLError:
            pass

    @staticmethod
    def currentweek(dbname, event_clean):
        """
        Send to plotly the currentweek tabel from the database
        update flag into the database if data uploaded
        :param event_clean from data_base
        :return:   none
        """
        try:
            conn = sqlite3.connect(dbname)

            with conn:

                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()

                # ****************************************************************************** #
                # Process for CurrentWeek Table
                # ****************************************************************************** #
                # Search the first rowid with uploaded = 0
                cur.execute('SELECT Count() FROM %s' % 'CurrentWeek')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %s' % count)
                    upl = int(cur.fetchone()['UPLOADED'])
                    count -= 1
                if upl == 0:
                    count_start = 1
                else:
                    if count+2 > count_end:
                        count_start = 0
                    else:
                        count_start = count + 2
                # end of search **********************************************

                # construct the differents list for stacked bar graph
                if count_start:
                    hp_range = []
                    hc_range = []
                    x1range = []

                    # code for Diff_HP Diff_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %s' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Hour']))
                        hp_range.append(data['Diff_HP'])
                        hc_range.append(data['Diff_HC'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(barmode='stack')
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        if event_clean:
                            py.plot(fig, filename='CurrentWeek_Diff', fileopt='overwrite', auto_open=False)
                        else:
                            py.plot(fig, filename='CurrentWeek_Diff', fileopt='extend', auto_open=False)
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in Diff methode plotly_currentweek()  ")
                        exit()

                    # code for Cumul_HPHC
                    cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %s' % count)
                    data = cur.fetchone()
                    x1range = str(data['Hour'])
                    hp_range = data['Cumul_HP']
                    hc_range = data['Cumul_HC']
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(title='Today Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                    xaxis=XAxis(title='Hour'))
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig, filename='Today_Cumul', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %s WHERE rowid = %s' % (1, count))
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in plotly_currentweek() ")
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_CurrentWeek()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def day(dbname):
        """
        Send to plotly the day table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM %s' % 'Day')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Day WHERE rowid = %s' % count)
                    upl = int(cur.fetchone()['UPLOADED'])
                    count -= 1
                if upl == 0:
                    count_start = 1
                else:
                    if count+2 > count_end:
                        count_start = 0
                    else:
                        count_start = count + 2
                # construct the differents list for stacked bar graph
                if count_start:
                    hp_range = []
                    hc_range = []
                    x1range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Day WHERE rowid = %s' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Day']) + "/" + str(data['Month']) + "/" + str(data['Year']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        cur.execute('UPDATE  Day SET UPLOADED = %s WHERE rowid = %s' % (1, count))

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(title='Days Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                    xaxis=XAxis(title='Date'))
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig, filename='Day', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %s WHERE rowid = %s' % (1, count))
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in plotly_day() ")
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_day()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def week(dbname):
        """
        Send to plotly the week table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM %s' % 'Week')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Week WHERE rowid = %s' % count)
                    upl = int(cur.fetchone()['UPLOADED'])
                    count -= 1
                if upl == 0:
                    count_start = 1
                else:
                    if count+2 > count_end:
                        count_start = 0
                    else:
                        count_start = count + 2
                # construct the differents list for stacked bar graph
                if count_start:
                    hp_range = []
                    hc_range = []
                    x1range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Week WHERE rowid = %s' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']) + "W" + str(data['Week_Number']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        cur.execute('UPDATE  Week SET UPLOADED = %s WHERE rowid = %s' % (1, count))

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(title='Week Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                    xaxis=XAxis(title='Date'))
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig, filename='Week', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %s WHERE rowid = %s' % (1, count))
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in plotly_week() ")
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_week()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def month(dbname):
        """
        Send to plotly the month table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM %s' % 'Month')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Month WHERE rowid = %s' % count)
                    upl = int(cur.fetchone()['UPLOADED'])
                    count -= 1
                if upl == 0:
                    count_start = 1
                else:
                    if count+2 > count_end:
                        count_start = 0
                    else:
                        count_start = count + 2
                # construct the differents list for stacked bar graph
                if count_start:
                    hp_range = []
                    hc_range = []
                    x1range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Month WHERE rowid = %s' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']) + "-" + str(data['Month']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        cur.execute('UPDATE  Month SET UPLOADED = %s WHERE rowid = %s' % (1, count))

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(title='Month Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                    xaxis=XAxis(title='Year-Month'))
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig, filename='Month', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %s WHERE rowid = %s' % (1, count))
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in plotly_month() ")
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_month()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def year(dbname):
        """
        Send to plotly the year table from the database
        update flag into the database if data uploaded
        :param
        :return:   none
        """
        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM %s' % 'Year')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Year WHERE rowid = %s' % count)
                    upl = int(cur.fetchone()['UPLOADED'])
                    count -= 1
                if upl == 0:
                    count_start = 1
                else:
                    if count+2 > count_end:
                        count_start = 0
                    else:
                        count_start = count + 2
                # construct the differents list for stacked bar graph
                if count_start:
                    hp_range = []
                    hc_range = []
                    x1range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Year WHERE rowid = %s' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        cur.execute('UPDATE  Year SET UPLOADED = %s WHERE rowid = %s' % (1, count))

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    dataobj = Data([trace1, trace2])
                    layout = Layout(title='Year Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                    xaxis=XAxis(title='Year'))
                    fig = Figure(data=dataobj, layout=layout)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig, filename='Year', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %s WHERE rowid = %s' % (1, count))
                    except exceptions.PlotlyConnectionError:
                        log_error("plotly error in plotly_year() ")
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_year()")
            exit()
            # print "Error %s:" % e.args[0]

