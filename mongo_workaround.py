import requests
import json
import pandas as pd
from dotenv import load_dotenv
import os

load_dotenv()
username = os.getenv('DRILLUSER')
password = os.getenv('DRILLPASS')

host = 'https://proxima.bigdata.fh-aachen.de:9095'
headers = {'Authorization': username+':'+password}
headers['Cache-Control'] = 'max-age=2'

query = """
select payload, `datetime` from mongo.mqtt.`waterquality/sensor` t
"""
data = {'query': query, 'format':"json"}
result = requests.post(host + '/query', json=data, headers=headers)

print(result)
j = json.loads(result.text)
df_unflattened = pd.DataFrame.from_dict(j)

df_unflattened['payload'] = df_unflattened['payload'].apply(json.loads)
df_flattened = df_unflattened['payload'].apply(pd.Series)
df_flattened['datetime'] = df_unflattened['datetime']
df_locations = df_flattened['l'].apply(pd.Series)
df_flattened["lx"] = df_locations['X']
df_flattened["ly"] = df_locations['Y']
df_flattened = df_flattened.drop("l",axis=1)


print(df_flattened)
