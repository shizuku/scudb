SELECT CompanyName, Round(CAST(a.l AS DOUBLE) / CAST(b.s AS DOUBLE) * 100, 2) AS Percent
FROM ((SELECT ShipVia, count(*) AS l
       FROM "Order"
       WHERE ShippedDate > RequiredDate
       GROUP BY ShipVia) AS a
    JOIN (SELECT ShipVia, count(*) AS s
          FROM "Order"
          GROUP BY ShipVia) AS b ON a.ShipVia = b.ShipVia)
         JOIN Shipper ON Shipper.Id = a.ShipVia
ORDER BY Percent DESC;
