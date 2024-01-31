import matplotlib.pyplot as plt
import pandas as pd
import requests
import json
from dotenv import load_dotenv
import os

load_dotenv()
username = os.getenv('DRILLUSER')
password = os.getenv('DRILLPASS')

host = 'https://proxima.bigdata.fh-aachen.de:9095'
headers = {'Authorization': username + ':' + password}
headers['Cache-Control'] = 'max-age=2'

query = """
select payload, `datetime` from mongo.mqtt.`waterquality/sensor` t
"""
data = {'query': query, 'format': "json"}
#result = requests.post(host + '/query', json=data, headers=headers)
result = requests.post(host + '/query', json=data, headers=headers, verify=False)
j = json.loads(result.text)
df_unflattened = pd.DataFrame.from_dict(j)

df_unflattened['payload'] = df_unflattened['payload'].apply(json.loads)
df_flattened = df_unflattened['payload'].apply(pd.Series)
df_flattened['datetime'] = df_unflattened['datetime']
df_locations = df_flattened['l'].apply(pd.Series)
df_flattened["lx"] = df_locations['X']
df_flattened["ly"] = df_locations['Y']
df_flattened = df_flattened.drop("l", axis=1)

df_flattened['datetime'] = pd.to_datetime(df_flattened['datetime'], format='"%d/%m/%Y %H:%M:%S"')

# Specific date and time range for filtering
# Define the timeframe for filtering
start_date = '2024-01-22'
end_date = '2024-01-24'
start_time = '15:40'
end_time = '15:39'
# Convert strings to datetime
start_datetime = pd.to_datetime(start_date + ' ' + start_time)
end_datetime = pd.to_datetime(end_date + ' ' + end_time)

# Filter the DataFrame for the specific date and time range
filtered_df = df_flattened[(df_flattened['datetime'] >= start_datetime) & (df_flattened['datetime'] <= end_datetime)]
filtered_df.to_csv('filtered_data_2.csv')
print(filtered_df.tail(1))  # Show the last element of the DataFrame
# Create subplots
fig, axs = plt.subplots(4, 1, figsize=(10, 20))
fig.suptitle(f'Sensor Data from {start_date} {start_time} to {end_date} {end_time}')

# Plot pH value
axs[0].plot(filtered_df['datetime'], filtered_df['p'])
axs[0].set_title('pH Value')
axs[0].set_xlabel('DateTime')
axs[0].set_ylabel('pH')

# Plot TDS value
axs[1].plot(filtered_df['datetime'], filtered_df['td'])
axs[1].set_title('TDS Value')
axs[1].set_xlabel('DateTime')
axs[1].set_ylabel('TDS')

# Plot ORP
axs[2].plot(filtered_df['datetime'], filtered_df['o'])
axs[2].set_title('ORP')
axs[2].set_xlabel('DateTime')
axs[2].set_ylabel('ORP')

# Plot Turbidity
axs[3].plot(filtered_df['datetime'], filtered_df['tu'])
axs[3].set_title('Turbidity')
axs[3].set_xlabel('DateTime')
axs[3].set_ylabel('Turbidity')

plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.show()
