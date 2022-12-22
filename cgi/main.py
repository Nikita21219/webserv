from datetime import datetime
import os
import sys

date = datetime.now().strftime('%d.%m.%Y - %H:%M:%S')
print(f'<h4>Date: {date}</h4>')
print('<ul>')
for field in os.environ:
    if field == 'QUERY_STRING':
        print(f'<li style="color: red">{field}={os.getenv(field)}</li>')
    elif field != 'MYSQL_PASS':
        print(f'<li style="color: blue">{field}={os.getenv(field)}</li>')
print('</ul>')
