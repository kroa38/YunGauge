#!/usr/bin/python
# -*- coding: utf-8 -*-

import sqlite3
import plotly.plotly as py
import plotly.tools as tls
import urllib2
import os
import time
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
    def plot(dbname, test):
        """
        Send to plotly the data
        :param dbname
        :return:   none
        """
        try:
            # for unlock websense (quiet and delete after download)
            if not test:
                os.system("wget -q --delete-after www.plot.ly")         # pour debloquer websense (quiet and delete after download.
                _ = urllib2.urlopen('https://plot.ly/', timeout=4)
                '''os.system("wget -q --delete-after www.google.fr")
                _ = urllib2.urlopen('http://www.google.fr/', timeout=4)'''

            PlotlyPlot.currentweek(dbname, test)
            time.sleep(5)
            PlotlyPlot.day(dbname, test)
            time.sleep(5)
            PlotlyPlot.week(dbname, test)
            time.sleep(5)
            PlotlyPlot.month(dbname, test)
            time.sleep(5)
            PlotlyPlot.year(dbname, test)

        except urllib2.URLError:
            pass

    @staticmethod
    def currentweek(dbname, test):
        """
        Send to plotly the currentweek tabel from the database
        update flag into the database if data uploaded
        :param dbname
        :return:   none
        """
        if test:
            teleinfo_folder = 'Test_Teleinfo/'
            tin_folder = 'Test_Temperature_In/'
        else:
            teleinfo_folder = 'Teleinfo/'
            tin_folder = 'Temperature_In/'

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
                cur.execute('SELECT Count() FROM CurrentWeek')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %d' % count)
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
                    tin_range = []
                    x1range = []

                    # code for Diff_HP Diff_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %d' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Hour']))
                        hp_range.append(data['Diff_HP'])
                        hc_range.append(data['Diff_HC'])
                        tin_range.append(data['Temp_In'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    trace3 = Scatter(x=x1range, y=tin_range, name='Temperature In', mode='lines+markers')
                    data_teleinfo = Data([trace1, trace2])
                    data_tin = Data([trace3])
                    layout_teleinfo = Layout(title='CurrentDay', barmode='stack', yaxis=YAxis(title='Watt'), xaxis=XAxis(title='Hour'))
                    layout_tin = Layout(title='Temperature In', yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    fig_tin = Figure(data=data_tin, layout=layout_tin)
                    requests.packages.urllib3.disable_warnings()
                    cur.execute('SELECT * FROM Event WHERE rowid = 1')
                    plotly_overwrite = cur.fetchone()['Plotly_Cw_Ovr']

                    try:
                        tls.get_credentials_file()
                        if plotly_overwrite:
                            py.plot(fig_teleinfo, filename=teleinfo_folder + 'CurrentDay', fileopt='overwrite', auto_open=False)
                            py.plot(fig_tin, filename=tin_folder + 'CurrentDay', fileopt='overwrite', auto_open=False)
                            cur.execute('UPDATE Event SET Plotly_Cw_Ovr = 0 WHERE rowid = 1')
                        else:
                            py.plot(fig_teleinfo, filename=teleinfo_folder + 'CurrentDay', fileopt='extend', auto_open=False)
                            py.plot(fig_tin, filename=tin_folder + 'CurrentDay', fileopt='extend', auto_open=False)
                    except exceptions, e:
                        log_error(str(e))
                        exit()

                    # code for Cumul_HPHC
                    cur.execute('SELECT * FROM CurrentWeek WHERE rowid = %d' % count)
                    data = cur.fetchone()
                    x1range = str(data['Hour'])
                    hp_range = data['Cumul_HP']
                    hc_range = data['Cumul_HC']
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    data_teleinfo = Data([trace1, trace2])
                    layout_teleinfo = Layout(title='Today Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                             xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig_teleinfo, filename=teleinfo_folder + 'Today_Cumul', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  CurrentWeek SET UPLOADED = %d WHERE rowid = %d' % (1, count))
                    except exceptions, e:
                        log_error(str(e))
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_CurrentWeek()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def day(dbname, test):
        """
        Send to plotly the day table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        if test:
            teleinfo_folder = 'Test_Teleinfo/'
            tin_folder = 'Test_Temperature_In/'
        else:
            teleinfo_folder = 'Teleinfo/'
            tin_folder = 'Temperature_In/'

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM Day')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Day WHERE rowid = %d' % count)
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
                    tin_min_range = []
                    tin_avg_range = []
                    tin_max_range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Day WHERE rowid = %d' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Day']) + "/" + str(data['Month']) + "/" + str(data['Year']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        tin_min_range.append(data['Temp_In_Min'])
                        tin_avg_range.append(data['Temp_In_Avg'])
                        tin_max_range.append(data['Temp_In_Max'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    trace3 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
                    trace4 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
                    trace5 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
                    data_teleinfo = Data([trace1, trace2])
                    data_tin = Data([trace3, trace4, trace5])
                    layout_teleinfo = Layout(title='Days Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                             xaxis=XAxis(title='Date'))
                    layout_tin = Layout(title='Temperature In', yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    fig_tin = Figure(data=data_tin, layout=layout_tin)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig_teleinfo, filename=teleinfo_folder + 'Day', fileopt='overwrite', auto_open=False)
                        py.plot(fig_tin, filename=tin_folder + 'Day', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  Day SET UPLOADED = %d WHERE rowid = %d' % (1, count))
                    except exceptions, e:
                        log_error(str(e))
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_day()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def week(dbname, test):
        """
        Send to plotly the week table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        if test:
            teleinfo_folder = 'Test_Teleinfo/'
            tin_folder = 'Test_Temperature_In/'
        else:
            teleinfo_folder = 'Teleinfo/'
            tin_folder = 'Temperature_In/'

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM Week')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Week WHERE rowid = %d' % count)
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
                    tin_min_range = []
                    tin_avg_range = []
                    tin_max_range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Week WHERE rowid = %d' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']) + "W" + str(data['Week_Number']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        tin_min_range.append(data['Temp_In_Min'])
                        tin_avg_range.append(data['Temp_In_Avg'])
                        tin_max_range.append(data['Temp_In_Max'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    trace3 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
                    trace4 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
                    trace5 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
                    data_teleinfo = Data([trace1, trace2])
                    data_tin = Data([trace3, trace4, trace5])
                    layout_teleinfo = Layout(title='Week Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                             xaxis=XAxis(title='Date'))
                    layout_tin = Layout(title='Temperature In', yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    fig_tin = Figure(data=data_tin, layout=layout_tin)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig_teleinfo, filename=teleinfo_folder + 'Week', fileopt='overwrite', auto_open=False)
                        py.plot(fig_tin, filename=tin_folder + 'Week', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE Week SET UPLOADED = %d WHERE rowid = %d' % (1, count))
                    except exceptions, e:
                        log_error(str(e))
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_week()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def month(dbname, test):
        """
        Send to plotly the month table from the database
        update flag into the database if data uploaded
        :return:   none
        """
        if test:
            teleinfo_folder = 'Test_Teleinfo/'
            tin_folder = 'Test_Temperature_In/'
        else:
            teleinfo_folder = 'Teleinfo/'
            tin_folder = 'Temperature_In/'

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM Month')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Month WHERE rowid = %d' % count)
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
                    tin_min_range = []
                    tin_avg_range = []
                    tin_max_range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Month WHERE rowid = %d' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']) + "-" + str(data['Month']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        tin_min_range.append(data['Temp_In_Min'])
                        tin_avg_range.append(data['Temp_In_Avg'])
                        tin_max_range.append(data['Temp_In_Max'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    trace3 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
                    trace4 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
                    trace5 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
                    data_teleinfo = Data([trace1, trace2])
                    data_tin = Data([trace3, trace4, trace5])
                    layout_teleinfo = Layout(title='Month Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                             xaxis=XAxis(title='Year-Month'))
                    layout_tin = Layout(title='Temperature In', yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    fig_tin = Figure(data=data_tin, layout=layout_tin)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig_teleinfo, filename=teleinfo_folder + 'Month', fileopt='overwrite', auto_open=False)
                        py.plot(fig_tin, filename=tin_folder + 'Month', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE Month SET UPLOADED = %d WHERE rowid = %d' % (1, count))
                    except exceptions, e:
                        log_error(str(e))
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_month()")
            exit()
            # print "Error %s:" % e.args[0]

    @staticmethod
    def year(dbname, test):
        """
        Send to plotly the year table from the database
        update flag into the database if data uploaded
        :param
        :return:   none
        """
        if test:
            teleinfo_folder = 'Test_Teleinfo/'
            tin_folder = 'Test_Temperature_In/'
        else:
            teleinfo_folder = 'Teleinfo/'
            tin_folder = 'Temperature_In/'

        try:
            conn = sqlite3.connect(dbname)

            with conn:
                # connect database in dictionary mode
                conn.row_factory = sqlite3.Row
                cur = conn.cursor()
                cur.execute('SELECT Count() FROM Year')
                count = cur.fetchone()[0]
                count_end = count
                upl = 0
                while upl == 0 and count > 0:
                    cur.execute('SELECT * FROM Year WHERE rowid = %d' % count)
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
                    tin_min_range = []
                    tin_avg_range = []
                    tin_max_range = []
                    # code for Cumul_HP Cumul_HC
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM Year WHERE rowid = %d' % count)
                        data = cur.fetchone()
                        x1range.append(str(data['Year']))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
                        tin_min_range.append(data['Temp_In_Min'])
                        tin_avg_range.append(data['Temp_In_Avg'])
                        tin_max_range.append(data['Temp_In_Max'])

                    # upload data list to plotly
                    trace1 = Bar(x=x1range, y=hp_range, name='HP')
                    trace2 = Bar(x=x1range, y=hc_range, name='HC')
                    trace3 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
                    trace4 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
                    trace5 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
                    data_teleinfo = Data([trace1, trace2])
                    data_tin = Data([trace3, trace4, trace5])
                    layout_teleinfo = Layout(title='Year Cumul', barmode='stack', yaxis=YAxis(title='Watt'),
                                             xaxis=XAxis(title='Year'))
                    layout_tin = Layout(title='Temperature In', yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
                    fig_teleinfo = Figure(data=data_teleinfo, layout=layout_teleinfo)
                    fig_tin = Figure(data=data_tin, layout=layout_tin)
                    requests.packages.urllib3.disable_warnings()
                    try:
                        tls.get_credentials_file()
                        py.plot(fig_teleinfo, filename=teleinfo_folder + 'Year', fileopt='overwrite', auto_open=False)
                        py.plot(fig_tin, filename=tin_folder + 'Year', fileopt='overwrite', auto_open=False)
                        for count in range(count_start, count_end+1):
                            cur.execute('UPDATE  Year SET UPLOADED = %d WHERE rowid = %d' % (1, count))
                    except exceptions, e:
                        log_error(str(e))
                        exit()
        except sqlite3.Error:
            log_error("Error database access in plotly_year()")
            exit()
            # print "Error %s:" % e.args[0]
