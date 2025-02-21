import os
import fitz  # PyMuPDF for PDF processing
import pandas as pd
import spacy
import medspacy
import numpy as np
from flask import Flask, request, render_template, redirect, url_for
from werkzeug.utils import secure_filename
from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score

# Load the medical NLP model
nlp = spacy.load("en_core_web_sm")  # Use 'md' instead of 'sm'

# Initialize Flask app
app = Flask(__name__)

UPLOAD_FOLDER = "uploads"
ALLOWED_EXTENSIONS = {"pdf", "txt", "csv"}
app.config["UPLOAD_FOLDER"] = UPLOAD_FOLDER

# Ensure upload folder exists
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

# Sample ML model for CSV data
def train_model():
    # Dummy dataset for training (replace with actual dataset)
    data = pd.read_csv("uterine_cancer_dataset.csv")
    X = data.drop(columns=["Cancer_Diagnosis"])
    y = data["Cancer_Diagnosis"]
    
    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    
    model = RandomForestClassifier(n_estimators=100, random_state=42)
    model.fit(X_train, y_train)
    
    # Validate model
    y_pred = model.predict(X_test)
    print(f"Model Accuracy: {accuracy_score(y_test, y_pred) * 100:.2f}%")
    
    return model

# Train model on startup
model = train_model()

# Function to check allowed file extensions
def allowed_file(filename):
    return "." in filename and filename.rsplit(".", 1)[1].lower() in ALLOWED_EXTENSIONS

# Function to extract text from PDFs
def extract_text_from_pdf(pdf_path):
    doc = fitz.open(pdf_path)
    text = "\n".join([page.get_text("text") for page in doc])
    return text

# Function to analyze text for medical terms
def analyze_text(text):
    doc = nlp(text)
    medical_terms = [ent.text for ent in doc.ents if ent.label_ in {"DISEASE", "SYMPTOM", "TREATMENT"}]
    
    uterine_cancer_symptoms = {
        "abnormal bleeding", "pelvic pain", "unusual discharge", "pain during intercourse",
        "weight loss", "fatigue", "urinary urgency"
    }
    
    detected_symptoms = set(term.lower() for term in medical_terms) & uterine_cancer_symptoms
    return detected_symptoms

@app.route("/", methods=["GET", "POST"])
def upload_file():
    if request.method == "POST":
        if "file" not in request.files:
            return redirect(request.url)

        file = request.files["file"]
        if file.filename == "":
            return redirect(request.url)

        if file and allowed_file(file.filename):
            filename = secure_filename(file.filename)
            filepath = os.path.join(app.config["UPLOAD_FOLDER"], filename)
            file.save(filepath)

            # Process file based on type
            file_ext = filename.rsplit(".", 1)[1].lower()

            if file_ext in ["pdf", "txt"]:
                text = extract_text_from_pdf(filepath) if file_ext == "pdf" else open(filepath, "r").read()
                detected_symptoms = analyze_text(text)
                return render_template("result.html", symptoms=detected_symptoms)

            elif file_ext == "csv":
                df = pd.read_csv(filepath)
                df.fillna(df.mean(), inplace=True)
                features = df.drop(columns=["Cancer_Diagnosis"], errors="ignore")

                # Predict using ML model
                predictions = model.predict(features)
                cancer_count = np.sum(predictions)
                return render_template("result.html", cancer_count=cancer_count, total=len(predictions))

    return render_template("index.html")

if __name__ == "__main__":
    app.run(debug=True)
