SELECT
    first_name,
    middle_name,
    last_name,
    email,
    ROLE,
    d.name AS department,
    photo_path
FROM
    users u
JOIN departments d ON
    u.department_id = d.id
WHERE
    u.id = :id
