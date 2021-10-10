select Region.RegionDescription, Employee.FirstName, Employee.LastName, max(Employee.BirthDate)
from Region
         join Territory on Region.Id = Territory.RegionId
         join EmployeeTerritory on EmployeeTerritory.TerritoryId = Territory.Id
         join Employee on EmployeeTerritory.EmployeeId = Employee.Id
group by Region.Id;
