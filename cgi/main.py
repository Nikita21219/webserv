from datetime import datetime
import os


def execute():
    date = datetime.now().strftime('%d.%m.%Y - %H:%M:%S')
    print(f'<h4>Date: {date}</h4>')
    query_string = os.getenv("QUERY_STRING")
    if query_string:
        try:
            res = 0
            args = query_string.split('&')
            for arg in args:
                key, val = arg.split('=')
                res += int(val)
            print(f'<h1>Result = {res}</h1>')
        except ValueError:
            print('<h1>Wrong arguments</h1>')


if __name__ == '__main__':
    execute()
