#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 16:40:25 2015

 install the following python package

 pip install httplib2
 pip install google-api-python-client

 first of all :
 create a credential file with the get_credential.py

 This project need the files :

 error.log
 event.log
 credentials.json
 index.csv

"""
import time             # lib pour gestion heure
import httplib2         # lib requette http
import base64
import gspread          # lib for google spreadsheet
import json             # lib pour fichiers json
import csv              # lib pour fichiers csv
import os.path          # lib pour test fichiers
import urllib2          # lib pour requettes internet

from apiclient.discovery import build
from apiclient.http import MediaFileUpload
from oauth2client.file import Storage
from apiclient import errors
from email.mime.text import MIMEText

DRIVE_SCOPE = 'https://www.googleapis.com/auth/drive '
GMAIL_SCOPE = 'https://mail.google.com/'
SHEET_SCOPE = 'https://spreadsheets.google.com/feeds'

test_folder = '0B9Yp8cxBtjfeflpyS0pUdi1pSm92eTd0SVBpbklRQzJPd1psMkR1RkI1azRoZkRfNEpZb2c1'

# --------------------------------------------------------------------------------------------------


def oauth2_build(scope):
    """  query oauth2 to google
    :itype : string (scope name)
    :rtype : none
    """
    storage = Storage('credentials.json')
    credentials = storage.get()

    http_auth = httplib2.Http()
    # credentials.refresh(http_auth)
    # storage.put(credentials)

    http_auth = credentials.authorize(http_auth)

    if scope == DRIVE_SCOPE:
        # print 'Drive scope Oauth'
        authorisation = build(serviceName='drive', version='v2', http=http_auth)
        return authorisation
    if scope == GMAIL_SCOPE:
        # print 'Gmail scope Oauth'
        authorisation = build(serviceName='gmail', version='v1', http=http_auth)
        return authorisation
    if scope == SHEET_SCOPE:
        # print 'Sheet scope Oauth'
        try:
            authorisation = gspread.authorize(credentials)
            # print 'Authorisation for SpreadSheets OK'
            return authorisation
        except gspread.AuthenticationError:
            log_error("Oauth Spreadsheet error")
            # print 'Authorisation failure'

# --------------------------------------------------------------------------------------------------


def drive_insert_file(file_name, folder_id):
    """  envoie un fichier à google drive
    :itype : string (the file name)
    :itype : string (id of folder)
    :rtype : none
    """
    if os.path.isfile(file_name):

        drive_service = oauth2_build(DRIVE_SCOPE)
        media_body = MediaFileUpload(file_name, mimetype='*/*', resumable=True)

        body = {
            'title': file_name,
            'mimeType': '*/*',
            'parents': [{"kind": "drive#filelink", "id": folder_id}]
        }

        try:
            filehandler = drive_service.files().insert(body=body, media_body=media_body).execute()
            file_id = filehandler['id']
            textmessage = 'File : %s uploaded .  ' % file_name + 'ID File is : %s' % file_id
            log_event(textmessage)
            # print textmessage
            return [file_id]
        except errors.HttpError, e:
            error = json.loads(e.content)
            error = error['error']['message']
            log_error("HttpError in function drive_insert_file() : " + error)
            # print 'An error occured: %s' % error
    else:
        log_error("file %s doesn't exist in function : drive_insert_file() : " % file_name)
        exit()
# --------------------------------------------------------------------------------------------------


def drive_delete_file(file_id):
    """  delete a file in drive
    :itype : string (id of the file)
    :rtype : none
    """
    drive_service = oauth2_build(DRIVE_SCOPE)

    try:
        drive_service.files().delete(fileId=file_id).execute()
        textmessage = 'File : %s deleted' % file_id
        log_event(textmessage)
    except errors.HttpError, e:
        error = json.loads(e.content)
        error = error['error']['message']
        log_error("HttpError in function : drive_delete_file() : " + error)
        exit()

# --------------------------------------------------------------------------------------------------


def print_files_in_folder(folder_id):
    """  affiche les id des fichiers du répertoire
    :itype : string of the folder id
    :rtype : none
    """
    drive_service = oauth2_build(DRIVE_SCOPE)
    page_token = None
    while True:
        try:
            param = {}
            if page_token:
                param['pageToken'] = page_token
            children = drive_service.children().list(folderId=folder_id, **param).execute()
            if not len(children['items']):
                log_error("No File in folder or bad folder_id in function :  print_files_in_folder() ")
                break
            for child in children.get('items', []):
                print child['id']
            #        file = service.files().get(fileId=child['id']).execute()
            #        print 'Title: %s' % file['title']
            page_token = children.get('nextPageToken')
            if not page_token:
                break
        except errors.HttpError, e:
            error = json.loads(e.content)
            error = error['error']['message']
            log_error("HttpError in function : print_files_in_folder() : " + error)
            break
    exit()
# --------------------------------------------------------------------------------------------------


def gmaillistmessage():
    """  liste les messages de la boite mail
    :itype : none
    :rtype : print the ID
    """
    gmail_service = oauth2_build(GMAIL_SCOPE)
    try:
        threads = gmail_service.users().threads().list(userId='me').execute()
        if threads['threads']:
            for thread in threads['threads']:
                print 'Thread ID: %s' % (thread['id'])
    except errors.HttpError, e:
        error = json.loads(e.content)
        error = error['error']['message']
        log_error("HttpError in function  : gmaillistmessage() : " + error)
        exit()
# --------------------------------------------------------------------------------------------------


def gmailsendmessage(message):
    """  envoie le message par email
    :itype : string text message
    :rtype : none
    """
    text_message = gmailcreatemessage(message)
    gmail_service = oauth2_build(GMAIL_SCOPE)

    try:
        message = (gmail_service.users().messages().send(userId='me', body=text_message)
                   .execute())
        # print 'Message Id: %s' % message['id']
        log_event("Email Message Id: %s sent" % message['id'])
    except errors.HttpError, e:
        error = json.loads(e.content)
        error = error['error']['message']
        log_error("HttpError in function : gmailsendmessage()" + error)
        exit()
# --------------------------------------------------------------------------------------------------


def gmailcreatemessage(message_text):
    """  creation du message à envoyer pour email
    :itype : string text message
    :rtype : string  raw encoded
    """
    data_email = get_json_data_from_file('email.json')
    message = MIMEText(message_text)
    message['to'] = data_email['dest_mail']
    message['from'] = data_email['source_mail']
    message['subject'] = "Yuno info"
    return {'raw': base64.b64encode(message.as_string())}

# --------------------------------------------------------------------------------------------------


def get_json_data_from_file(file_name):
    """ Return the data object from the  json file in a python dictionnary.
    :itype : file name
    :rtype : dictionnary
    """
    try:
        json_data = open(file_name).read()
    except IOError:
        log_error("IOError in function function : get_json_data_from_file()")
        exit()
    else:
        python_data = json.loads(json_data)
        return python_data

# --------------------------------------------------------------------------------------------------


def check_connectivity():
    """  test cnx internet
    :itype : None
    :rtype : int (1 = true 0 = false)
    """
    try:
        response=urllib2.urlopen('http://www.google.fr/',timeout=4)
        print "true"
        return 1
    except urllib2.URLError as err:
        print "false"
        return 0

# --------------------------------------------------------------------------------------------------


def log_error(error_message):
    """ log to the file error.log the current error with the date
    :itype : string message
    :rtype : None
    """
    now = str(time.strftime("%c"))
    f = open("error.log", "a")
    f.write(now + "    " + error_message + "\r")
    f.close()
# --------------------------------------------------------------------------------------------------


def log_event(event_message):
    """ log to the file event.log the current event with the date
    :itype : string message
    :rtype : None
    """
    now = str(time.strftime("%c"))
    f = open("event.log", "a")
    f.write(now + "    " + event_message + "\r")
    f.close()
# --------------------------------------------------------------------------------------------------


def printrowcvs():

    ifile = open('csvfile.csv', "r")
    reader = csv.reader(ifile)

    ligne = 0
    for row in reader:
        print 'ligne %s %s' % (ligne, row[7])
        ligne += 1

    ifile.close()

# --------------------------------------------------------------------------------------------------


def get_index():
    """ return the index read in the file indexfile.csv
    :itype : none
    :rtype : integer
    """
    try:
        ifile = open('indexfile.csv', "r")
        data = ifile.read()
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


# def worksheet():

    # sheet_service = oauth2_build(SHEET_SCOPE)
    # wks = sheet_service.open("teleinfo").sheet1
    # val = wks.acell('E1').value
    # print val
    # val = wks.cell(1,2).value
    # print val
    # wks.update_cell(1,6,'lalalalo !')

# --------------------------------------------------------------------------------------------------


if __name__ == '__main__':

      #drive_delete_file("0B9Yp8cxBtjfea2xiU3VEblRsaE0")
      #File_Id = drive_insert_file("teleinfo.log", test_folder)
      # print_files_in_folder(test_folder)
      gmaillistmessage()
      #gmailsendmessage("test de message")
      #check_connectivity()
      #print str(get_index())
      #put_index(12355)
      #print str(get_index())

