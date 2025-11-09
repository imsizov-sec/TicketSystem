SELECT
    relative_path
FROM
    ticket_files
WHERE
    ticket_id = :ticketId
    AND file_name = :fileName
