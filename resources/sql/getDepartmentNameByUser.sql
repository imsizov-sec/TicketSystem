SELECT
    d.name AS department_name
FROM
    ticket_system.departments d
JOIN ticket_system.users u ON 
    d.id = u.department_id
WHERE
    u.id = :userId
