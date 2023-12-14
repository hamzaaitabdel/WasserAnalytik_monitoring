import requests
import json
import pandas as pd
from dotenv import load_dotenv
import os
import matplotlib.pyplot as plt
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


df_flattened['datetime'] = pd.to_datetime(df_flattened['datetime'], format='"%d/%m/%Y %H:%M:%S"')

# Define your date range
start_date = '2023-11-30'  # Adjust to your desired start date
end_date = '2023-12-30'    # Adjust to your desired end date

# Filter the DataFrame
filtered_df = df_flattened[(df_flattened['datetime'] >= start_date) & (df_flattened['datetime'] <= end_date)]

# Plotting
plt.figure(figsize=(10, 6))
plt.plot(filtered_df['datetime'], filtered_df['p'])  # Assuming 'p' is the pH values
plt.xlabel('DateTime')
plt.ylabel('pH Value')
plt.title(f'pH Values from {start_date} to {end_date}')
plt.xticks(rotation=45)
plt.tight_layout()
plt.show()