select CompanyName, CustomerId, TotalExpenditures
from (SELECT *, ntile(4) over (order by TotalExpenditures) as n
      FROM (
               SELECT ifnull(CompanyName, 'MISSING_NAME') as CompanyName,
                      "Order".CustomerId,
                      round(sum(UnitPrice * Quantity), 2) as TotalExpenditures
               FROM "Order"
                        join OrderDetail on "Order".Id = OrderDetail.OrderId
                        left outer join "Customer" on Customer.Id = "Order".CustomerId
               group by "Order".CustomerId
               order by TotalExpenditures
           ))
where n = 1;

