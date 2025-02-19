# Uterine-cancer-Detection
This system uses ML and IoT for early uterine cancer detection. Biochemical sensors track estrogen &amp; progesterone levels, sending data via ESP8266/Raspberry Pi to the cloud. ML models analyze risks, triggering alerts for monitoring, prevention, and intervention.

//python code for analysis
import requests
import pandas as pd
import PyPDF2
import pytesseract
from PIL import Image
import io
from google.colab import files

# Upload Document
uploaded = files.upload()

# Function to extract text from PDFs
def extract_text_from_pdf(pdf_file):
    text = ""
    pdf_reader = PyPDF2.PdfReader(pdf_file)
    for page in pdf_reader.pages:
        text += page.extract_text() + "\n"
    return text

# Function to extract text from images
def extract_text_from_image(image_file):
    image = Image.open(image_file)
    text = pytesseract.image_to_string(image)
    return text

# Process Uploaded Files
document_text = ""
for file_name in uploaded.keys():
    if file_name.endswith(".pdf"):
        with open(file_name, "rb") as pdf_file:
            document_text += extract_text_from_pdf(pdf_file)
    elif file_name.endswith((".jpg", ".png", ".jpeg")):
        document_text += extract_text_from_image(file_name)
    elif file_name.endswith(".txt"):
        with open(file_name, "r", encoding="utf-8") as txt_file:
            document_text += txt_file.read()

# Display Extracted Text
print("\nExtracted Text from Document:\n")
print(document_text[:1000])  # Show only first 1000 characters

# Simple Analysis (Extracting Hormone Values)
import re
def extract_hormone_levels(text):
    hormones = {}
    patterns = {
        "Estrogen": r"Estrogen[:\s]*([\d\.]+)",
        "Progesterone": r"Progesterone[:\s]*([\d\.]+)",
        "Insulin": r"Insulin[:\s]*([\d\.]+)",
        "IGF-1": r"IGF-1[:\s]*([\d\.]+)"
    }
    for hormone, pattern in patterns.items():
        match = re.search(pattern, text, re.IGNORECASE)
        if match:
            hormones[hormone] = float(match.group(1))
    return hormones

extracted_hormones = extract_hormone_levels(document_text)
print("\nExtracted Hormone Levels from Document:")
print(extracted_hormones)

# Fetch Real-Time Data from ThingSpeak
THINGSPEAK_CHANNEL_ID = "YOUR_CHANNEL_ID"
THINGSPEAK_READ_API = "YOUR_READ_API_KEY"
URL = f"https://api.thingspeak.com/channels/{THINGSPEAK_CHANNEL_ID}/feeds.json?api_key={THINGSPEAK_READ_API}&results=1"

response = requests.get(URL)
data = response.json()
feeds = data['feeds'][0]

# Parse IoT Data
iot_data = {
    "Estrogen": float(feeds["field1"]),
    "Progesterone": float(feeds["field2"]),
    "Insulin": float(feeds["field3"]),
    "IGF-1": float(feeds["field4"])
}

print("\nReal-Time Sensor Data from ThingSpeak:")
print(iot_data)

# Combine Document & IoT Data for Risk Analysis
def analyze_risk(document_data, iot_data):
    risk_factors = []
    
    estrogen = document_data.get("Estrogen", iot_data["Estrogen"])
    progesterone = document_data.get("Progesterone", iot_data["Progesterone"])
    insulin = document_data.get("Insulin", iot_data["Insulin"])
    igf1 = document_data.get("IGF-1", iot_data["IGF-1"])

    if estrogen > 100 and progesterone < 5:
        risk_factors.append("⚠️ High Estrogen & Low Progesterone (Potential Cancer Risk)")
    if insulin > 20:
        risk_factors.append("⚠️ High Insulin (Risk of Metabolic Disorders)")
    if igf1 > 250:
        risk_factors.append("⚠️ High IGF-1 (Risk of Tumor Growth)")

    return risk_factors

risk_analysis = analyze_risk(extracted_hormones, iot_data)

print("\n⚠️ Risk Analysis:")
if risk_analysis:
    for risk in risk_analysis:
        print(risk)
else:
    print("✅ No significant hormonal imbalance detected.")

