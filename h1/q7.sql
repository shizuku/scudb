SELECT *, round(julianday(OrderDate) - julianday(PreviousOrderDate), 2) as diff
FROM (
         SELECT "Order".Id,
                "Order".OrderDate,
                lag("Order".OrderDate, 1, (SELECT DISTINCT "Order".OrderDate
                                           FROM Customer
                                                    JOIN "Order" ON Customer.Id = "Order".CustomerId
                                           WHERE Customer.Id = 'BLONP'
                                           ORDER BY "Order".OrderDate
                )) OVER (ORDER BY "Order".OrderDate) AS PreviousOrderDate
         FROM Customer
                  JOIN "Order" ON Customer.Id = "Order".CustomerId
         WHERE Customer.Id = 'BLONP'
         ORDER BY "Order".OrderDate
     )
;
