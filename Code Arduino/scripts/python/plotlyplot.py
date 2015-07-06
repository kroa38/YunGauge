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

        if PlotlyPlot.connect():
            tables = ['Hour', 'Day', 'Week', 'Month', 'Year']
            for table in tables:
                PlotlyPlot.teleinfo(dbname, table, telefolder)
                time.sleep(5)
                retval = PlotlyPlot.temperature(dbname, table, tempfolder)
                if retval:
                    SqlBase.clear_plotly_hour_ovr(dbname)
                SqlBase.set_flag_upload(dbname, table)
        else:
            pass


    @staticmethod
    def connect():
        """
        Connect to plotly
        :return:
        """
        try:
            # check if plotly website is online
            os.system("wget -q --delete-after www.plot.ly")         # quiet and delete after download.
            _ = urllib2.urlopen('https://plot.ly/', timeout=4)

            requests.packages.urllib3.disable_warnings()                # disable warning in plotly call with SSL
            tls.get_credentials_file()                    # connect to plotly
            return True
        except exceptions:
            return False

    @staticmethod
    def stackedbar():
        """
        this example don't work in plotly
        :return:
        """

        trace1 = Bar(
            x=['giraffes'],
            y=[132],
            name='SF Zoo'
        )
        trace2 = Bar(
            x=['giraffes'],
            y=[132],
            name='LA Zoo'
        )
        data = Data([trace1, trace2])
        layout = Layout(barmode='stack')
        fig = Figure(data=data, layout=layout)
        tls.get_credentials_file()
        plot_url = py.plot(fig, filename='stacked-bar', auto_open=False)



    @staticmethod
    def teleinfo(dbname, table, folder_name):
        """
        Send to plotly the currentweek tabel from the database
        update flag into the database if data uploaded
        :param dbname
        :return:   none
        """

        number_of_record = SqlBase.get_number_of_record(dbname)  # ignore first teleinfo record because plotly crash with double zero!
        if number_of_record < 2:
            return

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
                        if table == 'Hour':
                            hp_range.append(data['Diff_HP'])
                            hc_range.append(data['Diff_HC'])
                        else:
                            hp_range.append(data['Cumul_HP'])
                            hc_range.append(data['Cumul_HC'])
            except sqlite3.Error:
                log_error("Error database access in PlotlyPlot.teleinfo()")
                exit()
            # upload data list to plotly
            trace1 = Bar(y=hp_range, x=x1range, name='HP')
            trace2 = Bar(y=hc_range, x=x1range, name='HC')
            data = Data([trace1, trace2])
            layout = Layout(title=table, barmode='stack', yaxis=YAxis(title='Watt'), xaxis=XAxis(title=table))
            fig = Figure(data=data, layout=layout)
            if table == 'Hour':
                plotly_overwrite = SqlBase.get_plotly_cw_ovr(dbname)
            else:
                plotly_overwrite = 1
            try:
                if plotly_overwrite == 1:
                    py.plot(fig, filename=folder_name + table, fileopt='overwrite', auto_open=False)
                else:
                    py.plot(fig, filename=folder_name + table, fileopt='extend', auto_open=False)
                return plotly_overwrite
            except exceptions, e:
                log_error(str(e))
                exit()
        else:
            pass


    @staticmethod
    def temperature(dbname, table, folder_name):
        """
        Send to plotly the temperature
        :param dbname:
        :param table:
        :param folder_name:
        :return: plotly_overwrite
        """
        count_dict = SqlBase.count_to_upload(dbname, table)
        count_start = count_dict['count_start']
        count_end = count_dict['count_end']

        # construct the differents list for stacked bar graph
        if count_start:
            tin_range = []
            tin_min_range = []
            tin_avg_range = []
            tin_max_range = []
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
                        if table == 'Hour':
                            tin_range.append(data['Temp_In'])
                        else:
                            tin_min_range.append(data['Temp_In_Min'])
                            tin_avg_range.append(data['Temp_In_Avg'])
                            tin_max_range.append(data['Temp_In_Max'])
            except sqlite3.Error:
                log_error("Error database access in PlotlyPlot.temperature()")
                exit()
            # upload data list to plotly
            if table == 'Hour':
                plotly_overwrite = SqlBase.get_plotly_cw_ovr(dbname)
                trace = Scatter(x=x1range, y=tin_range, name='Temperature In', mode='lines+markers')
                data = Data([trace])
            else:
                plotly_overwrite = 1
                trace1 = Scatter(x=x1range, y=tin_min_range, name='Temperature In Min', mode='lines+markers')
                trace2 = Scatter(x=x1range, y=tin_avg_range, name='Temperature In Avg', mode='lines+markers')
                trace3 = Scatter(x=x1range, y=tin_max_range, name='Temperature In Max', mode='lines+markers')
                data = Data([trace1, trace2, trace3])
            layout = Layout(title=table, yaxis=YAxis(title='°C'), xaxis=XAxis(title=table))
            fig = Figure(data=data, layout=layout)
            try:
                if plotly_overwrite == 1:
                    py.plot(fig, filename=folder_name + table, fileopt='overwrite', auto_open=False)
                else:
                    py.plot(fig, filename=folder_name + table, fileopt='extend', auto_open=False)
                return plotly_overwrite
            except exceptions, e:
                log_error(str(e))
                exit()
        else:
            pass
