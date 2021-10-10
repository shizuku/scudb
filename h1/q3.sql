WITH is_na(Id, na) as (
    SELECT Id, 'NorthAmerica'
    FROM "Order"
    WHERE ShipCountry = 'USA'
       or ShipCountry = 'Mexico'
       or ShipCountry = 'Canada'
    UNION
    SELECT Id, 'OtherPlace'
    FROM "Order"
    WHERE NOT (ShipCountry = 'USA' or ShipCountry = 'Mexico' or ShipCountry = 'Canada')
)
SELECT "Order".Id, "Order".ShipCountry, is_na.na
FROM "Order"
         JOIN is_na ON "Order".Id = is_na.Id
WHERE "Order".Id >= 15445
ORDER BY "Order".Id
LIMIT 20;
