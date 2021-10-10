with recursive
    a(ProductId, ProductName) as (
        select *
        from (select ProductId, ProductName
              from "Order"
                       join Customer on Customer.Id = "Order".CustomerId
                       join "OrderDetail" on OrderDetail.OrderId = "Order".Id
                       join Product on Product.Id = OrderDetail.ProductId
              where CompanyName = 'Queen Cozinha'
                and "Order".OrderDate like '2014-12-25%'
              order by ProductId)
    ),
    c(ProductId, ProductName, n) as (
        select *, (select count(*) from a as b where a.ProductId > b.ProductId)
        from a
    ),
    d(ProductName, n) as (select ProductName, n from c),
    s(ProductNames, n) as (
        select *
        from d
        where n = 0
        union
        select s.ProductNames || ', ' || d.ProductName, d.n
        from d
                 join s on d.n = s.n + 1
    ),
    t (ProductNames, n, z) as (
        select *, 0
        from s
    ),
    f as (
        select ProductNames, max(n)
        from t
        group by z
    )
select ProductNames
from f;
