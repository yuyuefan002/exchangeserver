CREATE TABLE ACCOUNT(
ACCOUNT_ID INT PRIMARY KEY NOT NULL,
BALANCE DOUBLE PRECISION NOT NULL
);

CREATE TABLE POSITION(
POSITION_ID SERIAL PRIMARY KEY NOT NULL,
OWNER_ID INT REFERENCES ACCOUNT(ACCOUNT_ID),
SYM VARCHAR(20) NOT NULL,
SHARE DOUBLE PRECISION NOT NULL
);

CREATE TABLE ORDERS(
ORDER_ID SERIAL PRIMARY KEY NOT NULL,
ACCOUNT_ID INT REFERENCES ACCOUNT(ACCOUNT_ID),
SYM VARCHAR(20) NOT NULL,
STATUS char(2) NOT NULL DEFAULT 'op',
PRICE DOUBLE PRECISION NOT NULL,
SELL BOOLEAN NOT NULL,
TOTAL DOUBLE PRECISION NOT NULL,
REST DOUBLE PRECISION NOT NULL,
TM INT
);

CREATE TABLE EXECUTE(
EXECUTE_ID SERIAL PRIMARY KEY NOT NULL,
ORDER_ID INT REFERENCES ORDERS(ORDER_ID),
SHARE DOUBLE PRECISION NOT NULL,
PRICE DOUBLE PRECISION NOT NULL,
TIME INT NOT NULL
);
