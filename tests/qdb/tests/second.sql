select a.id, b.id, a.name, b.address
from authors a, publishers b
where a.id = 1 and a.id <= b.id;
