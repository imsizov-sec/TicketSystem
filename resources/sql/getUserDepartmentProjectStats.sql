SELECT
    pr.name AS project,
    COUNT(*) AS count
FROM
    tickets t
JOIN users u ON
    t.assignee_id = u.id
JOIN projects pr ON
    t.project_id = pr.id
WHERE
    u.department_id = (
        SELECT
            department_id
        FROM
            users
        WHERE
            id = :userId
    )
GROUP BY
    pr.name
ORDER BY
    pr.name
