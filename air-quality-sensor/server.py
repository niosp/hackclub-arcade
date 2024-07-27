from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import psycopg2
from psycopg2 import sql

DATABASE_URL = "postgresql://server:redactedPassword@10.10.10.253:5432/storage"

def get_db_connection():
    try:
        conn = psycopg2.connect(DATABASE_URL)
        return conn
    except Exception as e:
        print(f"Error connecting to the database: {e}")
        return None

# data model : (retrieved as json from server)
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
    pm1p0 = data.pm1p0
    pm2p5 = data.pm2p5
    pm4p0 = data.pm4p0
    pm10p0 = data.pm10p0
    temp = data.temp
    hum = data.hum
    nox = data.nox
    voc = data.voc

    print(f"PM1.0: {pm1p0}")
    print(f"PM2.5: {pm2p5}")
    print(f"PM4.0: {pm4p0}")
    print(f"PM10.0: {pm10p0}")
    print(f"Temperature: {temp}")
    print(f"Humidity: {hum}")
    print(f"NOx: {nox}")
    print(f"VOC: {voc}")

    # store values in database
    conn = get_db_connection()
    if conn is None:
        raise HTTPException(status_code=500, detail="Database connection failed")

    try:
        with conn.cursor() as cursor:
            insert_query = sql.SQL("""
                INSERT INTO storage (pm1p0, pm2p5, pm4p0, pm10p0, temp, hum, nox, voc)
                VALUES (%s, %s, %s, %s, %s, %s, %s, %s)
            """)
            cursor.execute(insert_query, (pm1p0, pm2p5, pm4p0, pm10p0, temp, hum, nox, voc))
            conn.commit()
    except Exception as e:
        conn.rollback()
        raise HTTPException(status_code=500, detail=f"Failed to insert data: {e}")
    finally:
        conn.close()

    return {"message": "Data received and stored successfully"}

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000)
