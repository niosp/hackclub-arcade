# script to check if the db already exists, if not, create it so the server can store the data later on
#!/bin/bash
DB_HOST="$POSTGRES_HOST"
DB_PORT="$POSTGRES_PORT"
DB_NAME="storage"
DB_USER="$POSTGRES_USER"
DB_PASSWORD="$POSTGRES_PASSWORD"
PGPASSWORD="$POSTGRES_PASSWORD"
SCHEMA_SQL="/server/schema.sql"

sleep 10s
DB_EXISTS=$(psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d postgres -tAc "SELECT 1 FROM pg_database WHERE datname='$DB_NAME'")

if [ "$DB_EXISTS" != "1" ]; then
    echo "Database '$DB_NAME' does not exist. Creating..."
    createdb -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" "$DB_NAME"
    psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f "$SCHEMA_SQL"
    python3 server.py
    echo "Database '$DB_NAME' created successfully. Starting application server..."
else
    echo "Database '$DB_NAME' already exists. Starting application server..."
    python3 server.py
fi