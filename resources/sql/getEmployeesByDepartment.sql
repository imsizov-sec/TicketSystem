SELECT
    u.id,
    u.first_name,
    u.last_name,
    u.middle_name,
    u.email,
    u.role,
    d.name AS department_name
FROM
    ticket_system.users u
JOIN ticket_system.departments d ON u.department_id = d.id
WHERE
    d.id = (
        SELECT department_id
        FROM ticket_system.users
        WHERE id = :userId
    )
    AND u.id != :userId
