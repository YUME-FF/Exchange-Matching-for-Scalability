CREATE TABLE IF NOT EXISTS ACCOUNT (
    ID          TEXT     PRIMARY KEY NOT NULL,
    BALANCE     FLOAT       NOT NULL
);

CREATE TABLE IF NOT EXISTS POSITION (
    ID    TEXT    REFERENCES ACCOUNT(ID),
    SYMBOL        TEXT    NOT NULL,
    AMOUNT        INT     NOT NULL
);

CREATE TABLE IF NOT EXISTS ORDERS (
    UNIQUE_ID         BIGINT          PRIMARY KEY     NOT NULL,
    ACCOUNT_ID        TEXT            REFERENCES ACCOUNT(ID),
    SYMBOL            TEXT            NOT NULL,
    AMOUNT            FLOAT          NOT NULL,
    LLIMIT             FLOAT           NOT NULL
);
CREATE TABLE IF NOT EXISTS EXECUTED (
    UNIQUE_ID    BIGINT             NOT NULL,
    ACCOUNT_ID        TEXT            REFERENCES ACCOUNT(ID),
    SYMBOL            TEXT            NOT NULL,
    AMOUNT            FLOAT          NOT NULL,
    PRICE             FLOAT           NOT NULL,
    TTIME              TEXT            NOT NULL
);

CREATE TABLE IF NOT EXISTS CANCELED (
    UNIQUE_ID    BIGINT       PRIMARY KEY     NOT NULL,
    ACCOUNT_ID        TEXT            REFERENCES ACCOUNT(ID),
    SYMBOL            TEXT            NOT NULL,
    AMOUNT            FLOAT          NOT NULL,
    TTIME              TEXT            NOT NULL
);

