import os
import fitz  # PyMuPDF for PDF processing
import pdfplumber
import pytesseract  # OCR for image extraction
import cv2  # OpenCV for image processing
import pandas as pd
import spacy
import numpy as np
import re
from flask import Flask, request, render_template
from werkzeug.utils import secure_filename

# Load NLP model
nlp = spacy.load("en_core_web_sm")

# Initialize Flask app
app = Flask(__name__)
UPLOAD_FOLDER = "uploads"
ALLOWED_EXTENSIONS = {"pdf", "jpeg", "jpg", "png"}
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

# Define cancer stages based on symptoms
STAGE_1 = {"abnormal bleeding", "unusual discharge"}
STAGE_2 = {"pelvic pain", "pain during intercourse"}
STAGE_3 = {"weight loss", "fatigue"}
STAGE_4 = {"urinary urgency", "severe pelvic pain"}

# Medical keywords to validate documents
MEDICAL_KEYWORDS = {"patient", "diagnosis", "doctor", "hospital", "test", "report", "hormone", "estrogen", "progesterone"}

# Define home remedies
HOME_REMEDIES = {
    "Stage 1 - Early Detection": ["Drink green tea", "Use turmeric", "Eat leafy greens"],
    "Stage 2 - Moderate Risk": ["Eat cruciferous vegetables", "Practice yoga", "Drink pomegranate juice"],
    "Stage 3 - Late Stage": ["Increase Omega-3 intake", "Use ginger & garlic", "Take vitamin C"],
    "Stage 4 - Advanced Cancer": ["Follow an anti-inflammatory diet", "Take Ashwagandha", "Stay hydrated"],
    "No signs of cancer detected.": ["Maintain a healthy diet", "Exercise regularly", "Practice meditation"]
}

# Check if file is allowed
def allowed_file(filename):
    return "." in filename and filename.rsplit(".", 1)[1].lower() in ALLOWED_EXTENSIONS

# Extract text from PDFs
def extract_text_from_pdf(pdf_path):
    doc = fitz.open(pdf_path)
    text = "\n".join([page.get_text("text") for page in doc])
    return text

# Extract text from images using OCR
def extract_text_from_image(image_path):
    image = cv2.imread(image_path)
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)  # Convert to grayscale
    text = pytesseract.image_to_string(gray)  # OCR to extract text
    return text

# Validate if a document is medical-related
def is_medical_document(text):
    words = set(re.findall(r"\b\w+\b", text.lower()))
    return bool(words & MEDICAL_KEYWORDS)

# Analyze symptoms and determine cancer stage
def analyze_text(text):
    doc = nlp(text)
    medical_terms = {ent.text.lower() for ent in doc.ents}
    detected_symptoms = (STAGE_1 | STAGE_2 | STAGE_3 | STAGE_4) & medical_terms

    if detected_symptoms.intersection(STAGE_4):
        stage = "Stage 4 - Advanced Cancer"
    elif detected_symptoms.intersection(STAGE_3):
        stage = "Stage 3 - Late Stage"
    elif detected_symptoms.intersection(STAGE_2):
        stage = "Stage 2 - Moderate Risk"
    elif detected_symptoms.intersection(STAGE_1):
        stage = "Stage 1 - Early Detection"
    else:
        stage = "No signs of cancer detected."

    remedies = HOME_REMEDIES.get(stage, [])
    return detected_symptoms, stage, remedies

@app.route("/", methods=["GET", "POST"])
def upload_file():
    detected_symptoms, cancer_stage, remedies, error_message = None, None, [], None

    if request.method == "POST":
        file = request.files["file"]
        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
            file.save(filepath)

            if filename.endswith(".pdf"):
                text = extract_text_from_pdf(filepath)

            elif filename.endswith((".jpeg", ".jpg", ".png")):
                text = extract_text_from_image(filepath)

            else:
                text = ""

            if not is_medical_document(text):
                error_message = "This file does not appear to be related to medical reports."
            else:
                detected_symptoms, cancer_stage, remedies = analyze_text(text)

    return render_template("result.html", symptoms=detected_symptoms, cancer_stage=cancer_stage, remedies=remedies, error_message=error_message)

if __name__ == "__main__":
    app.run(debug=True)
