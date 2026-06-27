import requests
import pymongo
from flask import Flask, jsonify
from flask_cors import CORS
from sklearn.linear_model import LinearRegression
import numpy as np

app = Flask(__name__)
CORS(app)

CHANNEL_ID = "3417488"
READ_API_KEY = "***"

client = pymongo.MongoClient("mongodb+srv://***:***@projectpdl.o4wmvyv.mongodb.net/?appName=ProjectPDL")
db = client["smart_city"]
collection = db["data"]

def fetch_and_store():
    url = f"https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds.json?api_key={READ_API_KEY}&results=50"
    try:
        data = requests.get(url, timeout=10).json()["feeds"]
        for entry in data:
            collection.update_one({"created_at": entry["created_at"]}, {"$set": entry}, upsert=True)
        return data
    except Exception as e:
        print(f"ThingSpeak inaccessible: {e}")
        data = list(collection.find({}, {"_id": 0}).sort("created_at", -1).limit(50))
        return data

@app.route("/api/data")
def get_data():
    data = fetch_and_store()
    return jsonify(data)

@app.route("/api/predict")
def predict():
    data = list(collection.find().sort("created_at", -1).limit(20))
    distances = [float(d["field2"]) for d in data if d.get("field2")]
    if len(distances) < 5:
        return jsonify({"prediction": "pas assez de donnees"})
    X = np.arange(len(distances)).reshape(-1, 1)
    y = np.array(distances)
    model = LinearRegression().fit(X, y)
    next_val = model.predict([[len(distances)]])[0]
    return jsonify({"prediction_distance": round(next_val, 2)})

if __name__ == "__main__":
    app.run(debug=True, port=5000)