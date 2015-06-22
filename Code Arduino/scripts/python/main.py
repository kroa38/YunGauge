#!/usr/bin/python
# -*- coding: utf-8 -*-


import random
from cloudscope import oauth2_build
from database import DataBase
from timefunc import TimeFunc
from plotlyplot import PlotlyPlot


DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive '
GMAIL_SCOPE = 'https://mail.google.com/'
SHEET_SCOPE = 'https://spreadsheets.google.com/feeds'
# DATABASE_NAME = '/root/python/database.db'
DATABASE_NAME = 'database.db'
index_hp = 5451113
index_hc = 5323021
epochdate = 1022401000


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


def datalist_test():
    """
    For test only this method return a list  like [1256234,1546879,"HP","2012-07-09T22:45:22"]
    the global variable a defined at the beginning of the file
    in : void
    Out list  [Int, Int, String, String]  [1451271, 1324002, 'HP', '2001-12-11T06:21:40']
    """

    global index_hc
    global index_hp
    global epochdate
    epochdate += 1800

    datetime_iso8601 = TimeFunc.epoch_to_iso8601(epochdate)
    heures = int(TimeFunc.epoch_to_hour(epochdate))
    minutes = int(TimeFunc.epoch_to_minute(epochdate))

    mode = "HP"
    # check between 15h38 and 17h38
    if (heures * 60 + minutes > 15 * 60 + 38) and (heures * 60 + minutes < 17 * 60 + 38):
        mode = "HC"
    # check between 21h08 and 23h59
    if (heures * 60 + minutes > 21 * 60 + 8) and (heures * 60 + minutes < 23 * 60 + 59):
        mode = "HC"
    # check between 00h00 and 03h08
    if (heures * 60 + minutes > 0) and (heures * 60 + minutes < 3 * 60 + 8):
        mode = "HC"
    if mode == "HP":
        index_hp += abs(random.randint(1, 100))  # increment HP during HP time
    if mode == "HC":
        index_hc += abs(random.randint(1, 100))  # increment HC during HC time

    liste1 = [index_hp, index_hc, mode, datetime_iso8601]
    #print liste1
    # liste2 = [index_hp, index_hc, mode]

    return liste1

if __name__ == '__main__':

    '''DataBase.resetflagupload()
    plotly_main(0)'''

    '''database_update(sys.argv[1:])
    upload_plotly()'''
    for x in range(0, 905):
        evt = DataBase.update(DATABASE_NAME, 1, datalist_test())

        #PlotlyPlot.plot(DATABASE_NAME, evt)

