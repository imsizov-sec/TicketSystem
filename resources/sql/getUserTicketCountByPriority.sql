SELECT
    p.name AS priority,
    COUNT(*) AS count
FROM
    tickets t
JOIN priorities p ON
    t.priority_id = p.id
WHERE
    t.assignee_id = :userId
GROUP BY
    p.name
ORDER BY
    p.name
