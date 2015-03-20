# work with gdata 2.018
import time
import gdata.spreadsheet.service
import urllib2  
from xml.etree.cElementTree import parse, dump
    
email = ''
password = ''
weight = '181'
# Find this value in the url with 'key=XXX' and copy XXX below
spreadsheet_key = '1aVGuAfXJ4l6Ey9IpLwltmWnG62c'
# All spreadsheets have worksheets. I think worksheet #1 by default always
# has a value of 'od6' and the next are 2, 3, 4, 5 .....
#worksheet_id = 'od6'
worksheet_id = '2'

spr_client = gdata.spreadsheet.service.SpreadsheetsService()
spr_client.email = 'email'
spr_client.password = 'password'
spr_client.source = 'Example Spreadsheet Writing Application'
spr_client.ProgrammaticLogin()


# Prepare the dictionary to write
dict = {}
dict['date'] = time.strftime('%m/%d/%Y')
dict['time'] = time.strftime('%H:%M:%S')
dict['weight'] = weight
print dict

entry = spr_client.InsertRow(dict, spreadsheet_key, worksheet_id)
print entry
if isinstance(entry, gdata.spreadsheet.SpreadsheetsList):
  print "Insert row succeeded."
else:
  print "Insert row failed."

