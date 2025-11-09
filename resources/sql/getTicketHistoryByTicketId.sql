SELECT
    u.full_name AS USER,
    th.changes_summary,
    th.comment,
    th.changed_at
FROM
    ticket_history th
JOIN users u ON
    th.user_id = u.id
WHERE
    th.ticket_id = :ticketId
ORDER BY
    th.changed_at DESC
