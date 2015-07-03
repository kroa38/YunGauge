#!/usr/bin/python
# -*- coding: utf-8 -*-

import sqlite3
import plotly.plotly as py
import plotly.tools as tls
import urllib2
import os
import requests  # Used for the warning InsecurePlatformWarning in python 2.7.3
from plotly import exceptions
from plotly.graph_objs import *
from cloudscope import log_error
from sqlcode import SqlBase


class PlotlyPlot:
    """ Class to access to plotly"""
    def __init__(self):

        pass

    @staticmethod
    def plot(dbname, test):
        """
        Send to plotly the data
        :param dbname
        :return:   none
        """

        if test:
            telefolder = 'Test_Teleinfo/'
            tempfolder = 'Test_Temperature/'
        else:
            telefolder = 'Yun_Teleinfo/'
            tempfolder = 'Yun_Temperature/'

        try:
            # for unlock websense (quiet and delete after download)
            if not test:
                os.system("wget -q --delete-after www.plot.ly")         # quiet and delete after download.
                _ = urllib2.urlopen('https://plot.ly/', timeout=4)

            #disable warning in plotly call with SSL
            requests.packages.urllib3.disable_warnings()

            PlotlyPlot.teleinfo_hour(dbname, 'Hour', telefolder)
            retval = PlotlyPlot.temperature_hour(dbname, 'Hour', tempfolder)
            if retval:
                SqlBase.clear_plotly_cw_ovr(dbname)

            PlotlyPlot.teleinfo(dbname, 'Day', telefolder)
            PlotlyPlot.temperature(dbname, 'Day', tempfolder)
            SqlBase.set_flag_upload(dbname, 'Day')

            PlotlyPlot.teleinfo(dbname, 'Week', telefolder)
            PlotlyPlot.temperature(dbname, 'Week', tempfolder)
            SqlBase.set_flag_upload(dbname, 'Week')

            PlotlyPlot.teleinfo(dbname, 'Month', telefolder)
            PlotlyPlot.temperature(dbname, 'Month', tempfolder)
            SqlBase.set_flag_upload(dbname, 'Month')

            PlotlyPlot.teleinfo(dbname, 'Year', telefolder)
            PlotlyPlot.temperature(dbname, 'Year', tempfolder)
            SqlBase.set_flag_upload(dbname, 'Year')

        except urllib2.URLError:
            pass

    @staticmethod
    def teleinfo_hour(dbname, table, folder_name):
        """
        Send to plotly the currentweek tabel from the database
        update flag into the database if data uploaded
        :param dbname
        :return:   none
        """
        count_dict = SqlBase.count_to_upload(dbname, table)
        count_start = count_dict['count_start']
        count_end = count_dict['count_end']

        # construct the differents list for stacked bar graph
        if count_start:
            hp_range = []
            hc_range = []
            x1range = []
            try:
                conn = sqlite3.connect(dbname)
                with conn:
                    conn.row_factory = sqlite3.Row
                    cur = conn.cursor()
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM %s WHERE rowid = %d' % (table, count))
                        data = cur.fetchone()
                        x1range.append(str(data[table]))
                        hp_range.append(data['Diff_HP'])
                        hc_range.append(data['Diff_HC'])
            except sqlite3.Error:
                log_error("Error database access in teleinfo_today_diff")
                exit()
            # upload data list to plotly
            trace1 = Bar(x=x1range, y=hp_range, name='HP')
            trace2 = Bar(x=x1range, y=hc_range, name='HC')
            data = Data([trace1, trace2])
            layout = Layout(title=table, barmode='stack', yaxis=YAxis(title='Watt'), xaxis=XAxis(title='Hour'))
            fig = Figure(data=data, layout=layout)
            plotly_overwrite = SqlBase.get_plotly_cw_ovr(dbname)
            try:
                tls.get_credentials_file()
                if plotly_overwrite == 1:
                    py.plot(fig, filename=folder_name + table, fileopt='overwrite', auto_open=False)
                    return plotly_overwrite
                else:
                    py.plot(fig, filename=folder_name + table, fileopt='extend', auto_open=False)
                    return plotly_overwrite
            except exceptions, e:
                log_error(str(e))
                exit()

    @staticmethod
    def temperature_hour(dbname, table, folder_name):
        """
        Send to plotly the temperature
        :param dbname:
        :param table:
        :param folder_name:
        :return:
        """
        count_dict = SqlBase.count_to_upload(dbname, table)
        count_start = count_dict['count_start']
        count_end = count_dict['count_end']

        # construct the differents list for stacked bar graph
        if count_start:
            tin_range = []
            x1range = []
            try:
                conn = sqlite3.connect(dbname)
                with conn:
                    conn.row_factory = sqlite3.Row
                    cur = conn.cursor()
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM %s WHERE rowid = %d' % (table, count))
                        data = cur.fetchone()
                        x1range.append(str(data[table]))
                        tin_range.append(data['Temp_In'])
            except sqlite3.Error:
                log_error("Error database access in temperature_hour_diff()")
                exit()
            # upload data list to plotly
            trace = Scatter(x=x1range, y=tin_range, name='Temperature In', mode='lines+markers')
            data = Data([trace])
            layout = Layout(title=table, yaxis=YAxis(title='°C'), xaxis=XAxis(title='Hour'))
            fig = Figure(data=data, layout=layout)
            plotly_overwrite = SqlBase.get_plotly_cw_ovr(dbname)
            try:
                tls.get_credentials_file()
                if plotly_overwrite == 1:
                    py.plot(fig, filename=folder_name + table, fileopt='overwrite', auto_open=False)
                    return plotly_overwrite
                else:
                    py.plot(fig, filename=folder_name + table, fileopt='extend', auto_open=False)
                    return plotly_overwrite
            except exceptions, e:
                log_error(str(e))
                exit()

            # code for Cumul_HPHC
            #This method send to plotly the real time cumul of the current day
            #don't use this method as it is .
            #use it for test case.

            '''
            cur.execute('SELECT * FROM %s WHERE rowid = %d' % (table, count))
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
            except exceptions, e:
                log_error(str(e))
                exit()
                '''


    @staticmethod
    def teleinfo(dbname, table, folder_name):
        """
        Send to plotly the data
        :param dbname:
        :param table:
        :param folder_name:
        :return: count_dict (dictionnary of count_start and count_end
        """
        count_dict = SqlBase.count_to_upload(dbname, table)
        count_start = count_dict['count_start']
        count_end = count_dict['count_end']

        # construct the differents list for stacked bar graph
        if count_start:
            hp_range = []
            hc_range = []
            x1range = []
            # code for Cumul_HP Cumul_HC
            try:
                conn = sqlite3.connect(dbname)
                with conn:
                    conn.row_factory = sqlite3.Row
                    cur = conn.cursor()
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM %s WHERE rowid = %d' % (table, count))
                        data = cur.fetchone()
                        x1range.append(str(data[table]))
                        hp_range.append(data['Cumul_HP'])
                        hc_range.append(data['Cumul_HC'])
            except sqlite3.Error:
                log_error("Error database access in plotly %s" % table)
                exit()
            # upload data list to plotly
            trace1 = Bar(x=x1range, y=hp_range, name='HP')
            trace2 = Bar(x=x1range, y=hc_range, name='HC')
            data = Data([trace1, trace2])
            layout = Layout(title=table, barmode='stack', yaxis=YAxis(title='Watt'), xaxis=XAxis(title=table))
            fig = Figure(data=data, layout=layout)
            requests.packages.urllib3.disable_warnings()
            try:
                tls.get_credentials_file()
                py.plot(fig, filename=folder_name + table, fileopt='overwrite', auto_open=False)
            except exceptions, e:
                log_error(str(e))
                exit()

    @staticmethod
    def temperature(dbname, table, folder_name):
        """
        Send to plotly the data
        :param dbname:
        :param table:
        :param folder_name:
        :return: count_dict (dictionnary of count_start and count_end
        """
        count_dict = SqlBase.count_to_upload(dbname, table)
        count_start = count_dict['count_start']
        count_end = count_dict['count_end']

        # construct the differents list for stacked bar graph
        if count_start:
            x1range = []
            tin_min_range = []
            tin_avg_range = []
            tin_max_range = []
            # code for Cumul_HP Cumul_HC
            try:
                conn = sqlite3.connect(dbname)
                with conn:
                    conn.row_factory = sqlite3.Row
                    cur = conn.cursor()
                    for count in range(count_start, count_end+1):
                        cur.execute('SELECT * FROM %s WHERE rowid = %d' % (table, count))
                        data = cur.fetchone()
                        x1range.append(str(data[table]))
                        tin_min_range.append(data['Temp_In_Min'])
                        tin_avg_range.append(data['Temp_In_Avg'])
                        tin_max_range.append(data['Temp_In_Max'])
            except sqlite3.Error:
                log_error("Error database access in plotly %s" % table)
                exit()
            # upload data list to plotly
            trace1 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
            trace2 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
            trace3 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
            data = Data([trace1, trace2, trace3])
            layout = Layout(title=table, barmode='stack', yaxis=YAxis(title='Temperature °C'), xaxis=XAxis(title=table))
            fig = Figure(data=data, layout=layout)
            requests.packages.urllib3.disable_warnings()
            try:
                tls.get_credentials_file()
                py.plot(fig, filename=folder_name + table, fileopt='extend', auto_open=False)
            except exceptions, e:
                log_error(str(e))
                exit()
