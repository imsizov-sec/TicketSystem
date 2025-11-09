SELECT
    file_name,
    relative_path
FROM
    ticket_files
WHERE
    ticket_id = :ticketId
