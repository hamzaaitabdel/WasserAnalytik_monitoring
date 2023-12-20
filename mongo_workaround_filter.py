import requests
import json
import pandas as pd
from datetime import datetime
from dotenv import load_dotenv
import os

# Load the environment variables
load_dotenv()
username = os.getenv('DRILLUSER')
password = os.getenv('DRILLPASS')

# Define host and headers for the API request
host = 'https://proxima.bigdata.fh-aachen.de:9095'
headers = {'Authorization': f"{username}:{password}"}
headers['Cache-Control'] = 'max-age=2'

# Define the SQL query to be sent to the API
query = """
select payload, `datetime` from mongo.mqtt.`waterquality/sensor` t
"""
# Define the request data format
data = {'query': query, 'format': "json"}

# Make the API request
result = requests.post(host + '/query', json=data, headers=headers)

# Check if the request was successful
if result.status_code == 200:
    j = json.loads(result.text)
    df_unflattened = pd.DataFrame.from_dict(j)

    # Convert the 'payload' column to json
    df_unflattened['payload'] = df_unflattened['payload'].apply(json.loads)
    df_flattened = df_unflattened['payload'].apply(pd.Series)

    # Convert the 'datetime' column to actual datetime objects
    df_flattened['datetime'] = pd.to_datetime(df_unflattened['datetime'])

    # Define the timeframe for filtering
    specific_date = '2023-12-16'
    start_time = '13:55'
    end_time = '14:36'

    # Convert strings to datetime
    start_datetime = pd.to_datetime(specific_date + ' ' + start_time)
    end_datetime = pd.to_datetime(specific_date + ' ' + end_time)
    filtered_df = df_flattened[(df_flattened['datetime'] >= start_datetime) & (df_flattened['datetime'] <= end_datetime)]
    # Display the filtered data
    print(filtered_df)
else:
    print("Failed to fetch data:", result.status_code)
