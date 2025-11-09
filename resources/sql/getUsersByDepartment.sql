SELECT
    id,
    full_name
FROM
    users
WHERE
    department_id = :departmentId
