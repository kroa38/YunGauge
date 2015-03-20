# -*- coding: utf-8 -*-
"""
Created on Tue Feb 17 16:40:25 2015

Ce script permet de récupérer les credentials à partir du fichier json
on utilise alors ce credential pour autoriser les appels API.
"""

from oauth2client.client import flow_from_clientsecrets
from oauth2client.file import Storage
import webbrowser

scope = 'https://www.googleapis.com/auth/drive https://mail.google.com https://spreadsheets.google.com/feeds '

# Run through the OAuth flow and retrieve credentials
flow = flow_from_clientsecrets(filename='client_secrets.json', scope=scope, redirect_uri='urn:ietf:wg:oauth:2.0:oob')

auth_uri = flow.step1_get_authorize_url()
webbrowser.open(auth_uri)

auth_code = raw_input('Enter the auth code: ')
credentials = flow.step2_exchange(auth_code)
storage = Storage('credentials.json')
storage.put(credentials)






