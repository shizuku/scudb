SELECT ProductName, CompanyName, ContactName
FROM (
         SELECT ProductName, CompanyName, ContactName, min(OrderDate)
         FROM "Order"
                  JOIN Customer ON "Order".CustomerId = Customer.Id
                  JOIN OrderDetail ON "Order".Id = OrderDetail.OrderId
                  JOIN Product ON OrderDetail.ProductId = Product.Id
         WHERE Discontinued = 1
         GROUP BY Product.Id
     )
ORDER BY ProductName;
