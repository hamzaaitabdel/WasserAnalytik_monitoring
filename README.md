# WasserAnalytik Monitoring

## Overview
The WasserAnalytik Monitoring repository is dedicated to the monitoring and analysis of water quality using ESP32 and Python scripts. It contains a collection of files that work together to collect, process, and store water quality data.
![Example Image](banner.png)


## Repository Structure

### ESP32_waterQuality_code/
This folder contains Arduino code for the ESP32 microcontroller.

#### [ESP32_waterQuality_code.ino](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/ESP32_waterQuality_code/ESP32_waterQuality_code.ino)
An Arduino sketch for the ESP32. This code is responsible for collecting water quality data from various sensors and sending this data to a server.

### [filtered_data.csv](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/filtered_data.csv)
A CSV file that contains filtered water quality data while Testing the Project. This data is the output the Python script, ready for analysis or reporting.

### Python Scripts
These scripts are used for handling data, interacting with a database, and processing the collected water quality data.

#### [mongo_workaround.py](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/mongo_workaround.py)
A Python script for interacting with a MongoDB database. It is used for storing, retrieving, and processing the water quality data collected by the ESP32.

#### [mongo_workaround_filter.py](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/mongo_workaround_filter.py)
Another Python script, an extension or variant of `mongo_workaround.py`, focused on filtering and processing the data stored in database.

### Configuration and Requirements

#### [package-lock.json](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/package-lock.json)
This file is used to lock the versions of Node.js package dependencies. It ensures that the same versions of the packages are used whenever the project is installed.

#### [requirements.txt](https://github.com/hamzaaitabdel/WasserAnalytik_monitoring/blob/master/requirements.txt)
Lists all the Python package dependencies required for the project. This file is used to easily install all required Python packages.

## Getting Started
## Getting Started

To get started with the WasserAnalytik Monitoring project, follow these steps:

1. **Create a `.env` File for Authentication:**
   - Create a file named `.env` in the root directory of the project.
   - Inside this file, add your authentication data to connect to the server. Use the following format:
     ```
     DRILLUSER=your_username
     DRILLPASS=your_password
     ```
   - Replace `your_username` and `your_password` with your actual credentials.

2. **Install Required Packages:**
   - Ensure you have Python installed on your system.
   - Install the required Python packages by running the following command in your terminal:
     ```
     pip install -r requirements.txt
     ```
   - This will install all the dependencies listed in the `requirements.txt` file.
