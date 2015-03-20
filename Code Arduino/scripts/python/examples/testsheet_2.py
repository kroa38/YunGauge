# -*- coding: utf-8 -*-


import gdata.spreadsheet.service

email = ''
password = ''
spreadsheet_key = '1aVGuAfXJ4l6Ey9IsdrbdJ9z45LnVVuOpLwltmWnG62c' # key param
worksheet_id = 'od6' # default

def main():
    client = gdata.spreadsheet.service.SpreadsheetsService()
    client.debug = True
    client.email = 'email'
    client.password = 'password'
    client.source = 'test client'
    client.ProgrammaticLogin()
    
    rows = []
    rows.append({'date':'0','time':'First','weight':'120'})
    
    for row in rows:
        try:
            client.InsertRow(row, spreadsheet_key, worksheet_id)
        except Exception as e:
            print e
    
    return
    
if __name__ == '__main__':
    main()