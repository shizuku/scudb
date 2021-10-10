SELECT CategoryName, a.n, a.uavg, a.umin, a.umax, a.tuo
FROM Category
         JOIN (SELECT CategoryId,
                      COUNT(*)                 AS n,
                      round(avg(UnitPrice), 2) AS uavg,
                      min(UnitPrice)           AS umin,
                      max(UnitPrice)           AS umax,
                      sum(UnitsOnOrder)        AS tuo
               FROM Product
               GROUP BY CategoryId) AS a ON Category.Id = a.CategoryId
WHERE a.n > 10
ORDER BY CategoryId;
