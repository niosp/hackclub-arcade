from fastapi import FastAPI
from pydantic import BaseModel

# Define the data model
class SensorData(BaseModel):
    pm1p0: float
    pm2p5: float
    pm4p0: float
    pm10p0: float
    temp: float
    hum: float
    nox: float
    voc: float

app = FastAPI()

@app.post("/data")
async def receive_data(data: SensorData):
    # Store the values in variables
    pm1p0 = data.pm1p0
    pm2p5 = data.pm2p5
    pm4p0 = data.pm4p0
    pm10p0 = data.pm10p0
    temp = data.temp
    hum = data.hum
    nox = data.nox
    voc = data.voc

    # Print the values to the console
    print(f"PM1.0: {pm1p0}")
    print(f"PM2.5: {pm2p5}")
    print(f"PM4.0: {pm4p0}")
    print(f"PM10.0: {pm10p0}")
    print(f"Temperature: {temp}")
    print(f"Humidity: {hum}")
    print(f"NOx: {nox}")
    print(f"VOC: {voc}")

    return {"message": "Data received successfully"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
